/** Main file that will be run on the server
 * Author: HookTurns
 * Date: 21/04/2017
 * Version: 1.0
 */

/**********************************SETUP**************************************/
var express = require("express");
var app = express();
var bodyParser = require('body-parser');
var cors = require('cors'); // cross-origin
var path = require("path");
app.use(bodyParser.urlencoded({extended: true}));
app.use(bodyParser.json());

var Database = require('./models/db.js');
app.set("view engine", "ejs");

/* static files */
app.use(express.static("assets"));
app.use('/nextramlive', express.static("nextram-angular/dist"));  // changed to point to distribution output

/* PTV, trams */
var PTV = require('./ptvApi.js');
var ptv = new PTV(1000824, 'c269558f-5915-11e6-a0ce-06f54b901f07');
var tramData = require("./assets/json/tramstops.json");

/* Google Auth Library */
var CLIENT_ID = "90710147687-8fbuvdh5836egvpujoud8dbtmojqj9d9.apps.googleusercontent.com";
var GoogleAuth = require('google-auth-library');
var auth = new GoogleAuth;
var client = new auth.OAuth2(CLIENT_ID, '', '');
// client.verifyIdToken(
//     token,
//     CLIENT_ID,
//     // Or, if multiple clients access the backend:
//     //[CLIENT_ID_1, CLIENT_ID_2, CLIENT_ID_3],
//     function(e, login) {
//       var payload = login.getPayload();
//       var userid = payload['sub'];
//       // If request specified a G Suite domain:
//       //var domain = payload['hd'];
//     });

/* ---------------------------- Mongoose schemas ---------------------------- */
var mongoose = require('mongoose');

var Crowdedness = Database.Crowdedness;
var Disruption = Database.Disruption;
var User = Database.User;

/***********************************PTV ROUTES********************************/

/* groups the departures by a combination of route and direction ID */
var groupByRouteDirectionID = function(ptvData) {
  var departures = ptvData.departures;
  var newDepartures = {};
  for (var i=0; i<departures.length; i++) {
    var key = departures[i].route_id + '-' + departures[i].direction_id;
    if (key in newDepartures) {
      newDepartures[key].push(departures[i]); // add to existing array
    }
    else {
      newDepartures[key] = [departures[i]]; // initialise new array
    }
  }

  return newDepartures;
}

// GET request. params - stopid: int
app.get("/departures", cors(), function(req, res) {
    var callback = function(error, response, body) {

        try {
          var body = JSON.parse(body);
        }
        catch (SyntaxError) { // JSON parsing failed
          res.json({status: 'error'});
          return;
        }


        /* list all the runIDs. We'll do an IN query with them later */
        var runIds = [];
        if (body) { // body: PTV response
          // get a list of route IDs
          var ptvData = body;
          var departures = ptvData.departures;
          for (let i=0; i<departures.length; i++) {
            runIds.push(departures[i].run_id);
          }
        }

        // Check status and error reporting before processing JSON
        if (!error && response.statusCode == 200) {
            // Check validity (only process JSON files, does not want website request)
            if (response.headers['content-type'] == 'text/html') res.json({status: 'error'});

            var runCrowdedness = {};
            var disruptions = {};
            var crowdednessDone = false;
            var disruptionDone = false;

            var callback = function() {
                var crowdData = {crowdedness: runCrowdedness, disruptions: disruptions}

                // Send back the result in json format
                if (body && crowdednessDone && disruptionDone) {  // ptvData has been loaded, done looking for crowdsourced info from our database
                    var toSend = {
                        status: "success",
                        stopID: stopID,
                        ptvData: body,
                        groupedDepts: groupByRouteDirectionID(body),
                        crowdSourcedDisruptions: crowdData,
                        routeGuide: null
                    }
                    res.json(toSend);
                }

            }

            // Get Crowdedness from database
            Crowdedness.find({runID: {$in: runIds}}, function(err, result) {
              console.log('Crowdedness data');
              console.log(err);
              console.log(result);
              // Iterate result and calculate the crowdedness for the requested stop id
              var total = 0;
              for (let i = 0; i < result.length; i++) {
                  // Compare based on run_id
                  // Create key if not exist in runCrowdedness object
                  if (!(result[i].runID in runCrowdedness)) {
                      runCrowdedness[result[i].runID] = {crowdedness: Number(result[i].crowdednessLevel), count: 1, average: 0};
                  }
                  // Else increment crowdedness level and count
                  else {
                      runCrowdedness[result[i].runID].crowdedness += Number(result[i].crowdednessLevel);
                      runCrowdedness[result[i].runID].count++;
                  }
              }

              // Iterate every run id in runCrowdednessObject and calculate the average of every run id
              for (var run in runCrowdedness) {
                runCrowdedness[run].average = Math.round(runCrowdedness[run].crowdedness / runCrowdedness[run].count);
                // Classify level of crowdedness
                if (runCrowdedness[run].average == 0) {
                    runCrowdedness[run]["class"] = "Empty";
                }
                else if (runCrowdedness[run].average == 1) {
                    runCrowdedness[run]["class"] = "Decent";
                }
                else if (runCrowdedness[run].average == 2) {
                    runCrowdedness[run]["class"] = "Full";
                }
                else if (runCrowdedness[run].average == 3) {
                    runCrowdedness[run]["class"] = "Overcrowded";
                }
              }

              crowdednessDone = true;
              callback();
            });


            // get all crowdsourced disruptions
            Disruption.find({runID: {$in: runIds}}, function(err, result) {
              console.log('Disruption data');
              console.log(err);
              console.log(result);

              for (let i=0; i<result.length; i++) {
                if (!(result[i].runID in disruptions)) {  // key does not exist, create
                    disruptions[result[i].runID] = [result[i].disruption];
                }
                else {
                  disruptions[result[i].runID].push(result[i].disruption);  // add to existing array
                }
              }

              disruptionDone = true;
              callback();

            });


        }
    }

    // Give an error if user does not put stopId as query parameter
    if (!req.query.stopid) {
       res.json({status: 'error'});
    }
    else {
       var stopID = req.query.stopid;
       ptv.departures(stopID, callback); // sample stopID: 2504
    }
});

/***********************************ROUTES************************************/
// Home
app.get("/", function(req, res) {
    res.render("index", {pageId: "home"});
});

// About
app.get("/about", function(req, res) {
    res.render("index", {pageId: "about"});
});

// How It Works
app.get("/how-it-works", function(req, res) {
    res.render("index", {pageId: "hiw"});
});

// Search
app.get("/search", function(req, res) {
    res.render("index", {pageId: "search"});
});

// Nextram
app.get("/nextram", function(req, res) {
    // Process query if user has given stop name on search bar
    if (req.query.search) {
        var stop_name = req.query.search;
        var stop_id;

        var isFound = false;
        // Find appropriate stop id
        for (var i = 0; i < tramData["stops"].length; i++) {
            if (tramData["stops"][i]["stop_name"].includes(stop_name)) {
                stop_id = tramData["stops"][i]["stop_id"];
                isFound = true;
                break;
            }
        }

        if (!isFound) {
          console.log('not found');
          res.redirect('/search');
        }
        else {
          res.redirect("/nextram?stop_id=" + stop_id);
        }

    }
    else {
       res.sendFile(__dirname + "/nextram-angular/dist/index.html")
    }
});

// // Route Guide
// app.get("/route-guide", function(req, res) {
//   var info = require('../PTV/tram_routes/96.json');
//   var route96_stop = info.stops;
//   res.render("index", {
//     pageId: "route_guide",
//     route96Data: route96_stop
//   });
// });

/*********************************POST****************************************/


// Information gather from nextram page
app.post("/nextramdb", function(req, res) {
    var token = req.body.token;

    // verify with Google API token
    client.verifyIdToken(
      req.body.token,
      CLIENT_ID,
      function(error, login) {
        var payload = login.getPayload();
        var userid = payload['sub'];
        if (userid) {
          var userInput = {
            "runID": req.body.run_id,
            "stopID": req.body.stop_id,
            "crowdednessLevel": req.body.crowdedness,
            "userID": userid
          };
          Crowdedness.create(userInput, function(err, object) {
              if (err) {
                  console.log("Error");
                  res.json({status: 'fail', reason: 'Database error occurred.'});
              }
              else {
                  console.log("Insertion success" + object);
                  res.json({status: 'success'});
              }
          });

          // create a User if he/she does not exist in our DB yet
          User.find({'userID': userid}, function(err, results) {
            console.log('User.find');
            if (err) console.log(err);
            console.log(results);
            if (results.length == 0) {
              User.create({
                userID: userid,
                email: payload['email'],
                firstName: payload['given_name'],
                lastName: payload['family_name']
              }, (err, results) => {
                console.log(err)
                console.log(results)
              })
            }
          });
        }
        else {  // invalid token
          res.json({status: 'fail', reason: 'Invalid Google login token.'});
        }
      }
    )

});

// Create new disruption
app.post('/reportdisruption', function(req, res) {
  var token = req.body.token;

  // verify with Google API token
  client.verifyIdToken(
    req.body.token,
    CLIENT_ID,
    function(error, login) {
      var payload = login.getPayload();
      var userid = payload['sub'];
      if (userid) {
        var disruption = new Disruption({
          "status": req.body.status,
          "runID": req.body.runID,
          "stopID": req.body.stopID,
          "disruption": req.body.disruption,
          "userID": userid
        });

        disruption.save(function(err, newDisruption) {
            if (!err) {
                console.log('Insertion success' + newDisruption);
                res.json({status: 'success'});
            }
            else {
              console.log("Error");
              res.json({status: 'fail', reason: 'Database error occurred.'});
            }
        });

        // create a User if he/she does not exist in our DB yet
        User.find({'userID': userid}, function(err, results) {
          console.log('User.find');
          if (err) console.log(err);
          console.log(results);
          if (results.length == 0) {
            User.create({
              userID: userid,
              email: payload['email'],
              firstName: payload['given_name'],
              lastName: payload['family_name']
            }, (err, results) => {
              console.log(err)
              console.log(results)
            });
          }
        });
      }
      else {  // invalid token
        res.json({status: 'fail', reason: 'Invalid Google login token.'});
      }
    }
  )


});

// route to handle sign in. We'll use the userid
app.post('/tokensignin', function(req, res) {
  console.log(req.body);  // from body-parser
  if (req.body.idtoken) {
    client.verifyIdToken(
      req.body.idtoken,
      CLIENT_ID,
      function(error, login) {
        var payload = login.getPayload();
        console.log(payload);
        var userid = payload['sub'];
      }
    )
  }
})

/**********************************LISTEN*************************************/
// 404 Page Not Found
app.get("*", function(req, res) {
    res.render("index", {pageId: "404"});
});

// app.listen(3000, "localhost", function(req, res) {
app.listen(3000, function(req, res) {
  console.log("HookTurns server has started...")
});
