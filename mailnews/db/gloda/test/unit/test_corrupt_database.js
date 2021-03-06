/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * This test does not use glodaTestHelper because:
 * 1) We need to do things as part of the test without gloda having remotely
 *    thought about opening the database.
 * 2) We expect and desire that the logger produce a warning and glodaTestHelper
 *    takes the view that warnings = death.
 *
 * We do use the rest of the test infrastructure though.
 */

load("../../../../resources/mailDirService.js");
load("../../../../resources/mailTestUtils.js");
load("../../../../resources/logHelper.js");
load("../../../../resources/asyncTestUtils.js");

// -- Do configure the gloda prefs though...
const gPrefs = Cc["@mozilla.org/preferences-service;1"]
                 .getService(Ci.nsIPrefBranch);
// yes to indexing
gPrefs.setBoolPref("mailnews.database.global.indexer.enabled", true);
// no to a sweep we don't control
gPrefs.setBoolPref("mailnews.database.global.indexer.perform_initial_sweep",
    false);
// yes to debug output
gPrefs.setBoolPref("mailnews.database.global.logging.dump", true);


// -- Add a logger listener that throws when we give it a warning/error.
Components.utils.import("resource:///modules/gloda/log4moz.js");

/**
 * Count the type of each severity level observed.
 */
function CountingAppender() {
  this._name = "CountingAppender";
  this.counts = {};
}
CountingAppender.prototype = {
  reset: function CountingAppender_reset() {
    this.counts = {};
  },
  append: function CountingAppender_append(message) {
    if (!(message.level in this.counts))
      this.counts[message.level] = 1;
    else
      this.counts[message.level]++;
  },
  getCountForLevel: function CountingAppender_getCountForLevel(level) {
    if (level in this.counts)
      return this.counts[level];
    return 0;
  },
  toString: function() {
    return "One, two, three! Ah ah ah!";
  }
};


let countingAppender = new CountingAppender();
Log4Moz.repository.rootLogger.addAppender(countingAppender);

/**
 * Create an illegal=corrupt database and make sure that we log a message and
 * still end up happy.
 */
function test_corrupt_databases_get_reported_and_blown_away() {
  // - get the file path
  let dirService = Cc["@mozilla.org/file/directory_service;1"]
                     .getService(Ci.nsIProperties);
  let dbFile = dirService.get("ProfD", Ci.nsIFile);
  dbFile.append("global-messages-db.sqlite");

  // - protect dangerous people from themselves
  // (There should not be a database at this point; if there is one, we are
  // not in the sandbox profile we expect.  I wouldn't bother except we're
  // going out of our way to write gibberish whereas gloda accidentally
  // opening a valid database is bad but not horrible.)
  if (dbFile.exists())
    do_throw("There should not be a database at this point.");

  // - create the file
  mark_sub_test_start("creating gibberish file");
  let ostream = Cc["@mozilla.org/network/file-output-stream;1"]
                  .createInstance(Ci.nsIFileOutputStream);
  ostream.init(dbFile, -1, -1, 0);
  let fileContents = "I'm in ur database not being a database.\n";
  ostream.write(fileContents, fileContents.length);
  ostream.close();

  // - reset counts in preparation of gloda init
  countingAppender.reset();

  // - init gloda, get warnings
  mark_sub_test_start("init gloda");
  Components.utils.import("resource:///modules/gloda/public.js");
  mark_sub_test_start("gloda inited, checking");

  mark_action("actual", "Counting appender counts", [countingAppender.counts]);
  // we expect 2 warnings
  do_check_eq(countingAppender.getCountForLevel(Log4Moz.Level.Warn), 2);
  // and no errors
  do_check_eq(countingAppender.getCountForLevel(Log4Moz.Level.Error), 0);

  // - make sure the datastore has an actual database
  Components.utils.import("resource:///modules/gloda/datastore.js");

  if (!GlodaDatastore.asyncConnection)
    do_throw("No database connection suggests no database!");
}

var tests = [
  test_corrupt_databases_get_reported_and_blown_away,
];

function run_test() {
  async_run_tests(tests);
}
