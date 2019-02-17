/** This file contains all the main functions */

// ============================================================================
// "Top level" functions, i.e. functions that are called directly from the
// HTML or triggered from there.
// ============================================================================

// todo: maybe have a normal landing page in the future and just use loadSelectedRevisions instead?
/**
 * This function gets called from the main page validation.html at the
 * beginning and sets up the * page with the initial selection of revisions.
 * @param revString
 * @param revList
 */
function loadRevisions(revString, revList) {
    if (typeof revString === 'undefined') {
        // fixme: shouldn't that be an empty string?
        revString = null;
    }
    // fixme: this was a workaround for default values. But shouldn't rev list then also have a default value? Also note that JS support default values!

    console.log("Loading revisions from server");
    let rev_load_path = "../revisions";

    $.get(rev_load_path).then(function (data) {
        console.log("Loading done!");

        function setupRevisionLoader(ractive) {

            // load the defaults for the first time
            if (revString == null) {
                loadSelectedRevisions(data);
            } else {
                // otherwise, load a specific selection
                setupRactiveFromRevision(data, revString, revList);
            }

            // be ready to load any other revision configuration if user desires
            ractive.on('loadSelectedRevisions', function () {

                loadSelectedRevisions(data);
            });
        }

        setupRactive("revision", '#revisions', data, null, setupRevisionLoader);
    });
}

/** Gets called from the main html page and sets up a small system information
 * submenu.
 */
function setSystemInfo() {
    console.log("Setting system info.");
    $.get("../system_info").done(
        function(data) {
            setupRactive("system_info", '#systeminfo', data);
        }

    ).fail(
        function (){
            console.warn("Could not get system info.")
        }
    );
}

/**
 * Gets called from plot_container, if plot is clicked.
 * the popup must be installed when the user clicks the item. If the popup
 * is installed once Ractive is done creating the template the popups do not work
 * any more if the user clicked on the "Overview" checkbox because new DOM items
 * get created
 * @param item_id
 */
function triggerPopup(item_id) {

    $(`#${item_id}`).magnificPopup({
        type: 'inline',
        // Allow opening popup on middle mouse click. Always set it to true if
        // you don't provide alternative source in href.
        midClick: true
    });
}


/**
 * This function call is triggered by the button under the revisions list
 * "Load selected" and sets up the page with the new set of revisions.
 * @param data revision data
 */
function loadSelectedRevisions(data) {

    let revString = getSelectedRevsString();
    let revList = getSelectedRevsList();

    if (revString === "") {
        alert("Please select at least one tag!");
    }

    console.log(`Loading rev via string '${revString}'.`);

    setupRactiveFromRevision(data, revString, revList);
}

// ============================================================================
// Loading
// ============================================================================

/**
 * Gets information about the comparisons and plots (generated when
 * we generate the plots), merges it with the information about the revisions
 * and uses that to set up the package template.
 * If we cannot get the comparison/plot information, then the plots for the
 * current selection of revisions haven't yet been generated and we
 * request them.
 * @param revData
 * @param revString
 * @param revList
 */
function setupRactiveFromRevision(revData, revString, revList) {

    // don't event attempt to show comparisons for empty revisions
    if (revString === "")
        return;

    // make dynamic
    let comparisonLoadPath = `../comparisons/${revString}`;
    let createComparisonUrl = "../create_comparison";

    console.log(`Loading Comparison 'comparisonLoadPath'`);

    // todo: This SCREAMS to be refactored in some way....
    $.get(comparisonLoadPath).done(function (data) {

        // Get the newest revision within the selection
        // to get information about failed scripts and the
        // log files
        let newestRev = getNewestRevision(revData);

        console.debug(`Newest revision is '${newestRev["label"]}'`);

        // enrich the comparison data with the newest revision in this comparison
        data["newest_revision"] = newestRev;

        // We have two sources of information for scripts and plots:
        // * The comparison object from comparisonLoadPath
        // * The revision object
        // We update the data from the comparison object with additional data
        // from the revision object.
        if (newestRev != null) {
            console.debug("Updating package information.");

            // We now go through all of the packages in the revision object
            // and add the corresponding information to the comparison object.
            // object. For this we create a lookup table
            //    'package name' -> 'index in list'
            // for the comparison object.
            let comparisonDataPkg2Index = {};
            for (let index in data["packages"]) {
                let name = data["packages"][index]["name"];
                comparisonDataPkg2Index[name] = index;
            }

            for (let irev in newestRev["packages"]) {

                // Information to be copied from the revision object:
                let name = newestRev["packages"][irev]["name"];
                let failCount = newestRev["packages"][irev]["fail_count"];
                let scriptfiles = newestRev["packages"][irev]["scriptfiles"];
                let label = newestRev["label"];

                if (name in comparisonDataPkg2Index) {
                    // Found the package in the comparison object
                    // ==> Just add the information
                    let ipkg = comparisonDataPkg2Index [name];

                    data["packages"][ipkg]["fail_count"] = failCount;
                    data["packages"][ipkg]["scriptfiles"] = scriptfiles;
                    // Also store the label of the newest revision as this
                    // is needed to stich together the loading path of
                    // log files
                    data["packages"][ipkg]["newest_revision"] = label;
                } else {
                    // Did not find the package in the comparison object
                    // ==> If there's a reason to display it on the homepage
                    //     (e.g. failed scripts whose logs we want to make
                    //     available, then we need to add a new item to the
                    //     package list of the comparison object).
                    console.debug(
                        `Package '${newestRev["packages"][irev]["name"]}` +
                        "' was found in the revision file, but not in the" +
                        "comparison file. Probably this package did not " +
                        "create a single output file."
                    );
                    if (newestRev["packages"][irev]["fail_count"] > 0) {
                        console.debug(
                            "However it did have failing scripts, so we " +
                            "will make it visible on the validation page. "
                        );
                        // Create a new empty entry with the same information
                        // as above and add it to the data
                        let pkgDict = {};
                        pkgDict["name"] = name;
                        pkgDict["fail_count"] = failCount;
                        pkgDict["scriptfiles"] = scriptfiles;
                        pkgDict["newest_revision"] = label;
                        // Also add keys that usually come from the
                        // comparison file:
                        pkgDict["visible"] = true;
                        data["packages"].push(pkgDict);

                    }
                }
            }
        } else {
            console.debug("Newest rev is null.")
        }

        setupRactive("package", '#packages', data,
            // Wire the clicks on package names
            function (ractive) {

                if ("packages" in data) {
                    // todo: load the package which was last time viewn by the users
                    let firstPackageName = getDefaultPackageName(data["packages"]);
                    if (firstPackageName !== false) {
                        loadValidationPlots(firstPackageName, data);
                    } else {
                        console.warn("No package could be loaded.")
                    }
                }
                ractive.on({
                    load_validation_plots: function (evt) {
                        // This gets called if the user clicks on a package in the
                        // package-selection side menu.

                        // Hide all sub-packages
                        ractive.set('packages.*.display_setting', 'none');

                        let pkgs = ractive.get('packages');

                        // Display sub-packages for this one.
                        if (pkgs != null) {
                            for (let ipkg in pkgs) {
                                if (pkgs[ipkg].name === evt.context.name) {
                                    // disaplay this one
                                    ractive.set(`packages.${ipkg}.display_setting`, 'block');
                                    break;
                                }
                            }

                            // the context will contain the json object which was
                            // used to create this template instance
                            loadValidationPlots(evt.context.name, data);
                        }
                    }
                });
            });
    }).fail(function () {

        console.log(`Comparison '${revString}' does not exist yet, requesting it`);

        $.ajax({
            url: createComparisonUrl,
            type: "POST",
            contentType: "application/json",
            dataType: "json",
            data: JSON.stringify({
                "revision_list": revList
            })
        }).done(function (data) {
            let key = data["progress_key"];
            beginCreatePlotWait(revString, revList, key, revData);
        });
    });
}


/**
 * Sets up the plot containers with the correct plots corresponding to the
 * selection of the revisions.
 * @param packageLoadName
 * @param data
 */
function loadValidationPlots(packageLoadName, data) {
    console.log(`loadValidationPlots: Loading plots for package '${packageLoadName}'`);

    let loadedPackage = null;

    console.log(`loadValidationPlots: Comparison data for package '${packageLoadName}' loaded`);

    let selected_list = getSelectedRevsList();
    // update the already displayed revision labels with the correct colors
    $(".revision-label").each(function () {

        let label = $(this).text();
        // find the revision with the same label
        for (let i in data["revisions"]) {
            if (data["revisions"][i].label === label) {
                $(this).css("color", data["revisions"][i].color);
            }
        }

        if (selected_list.indexOf(label) < 0) {
            // the one which are not selected will be grayed out
            $(this).css("color", "grey");
        }
    });

    if (packageLoadName === "") {
        packageLoadName = getDefaultPackageName(data["packages"]);
    }

    // Find data of the package by package name
    for (let i in data["packages"]) {
        if (data["packages"][i].name === packageLoadName) {
            loadedPackage = data["packages"][i];
            break;
        }
    }

    // fixme: Shouldn't something happen here??
    if (loadedPackage == null) {
    }

    // create unique ids for each plot, which can be used to create
    // links to individual plot images
    let uniq_plot_id = 1;
    for (let i in loadedPackage["plotfiles"]) {
        for (let ploti in loadedPackage["plotfiles"][i]["plots"]) {
            loadedPackage["plotfiles"][i]["plots"][ploti]["unique_id"] = uniq_plot_id++;
        }
    }

    let wrappedPackage = {packages: [loadedPackage]};

    setupRactive("plot_container", '#content', wrappedPackage, null,
        // on complete
        function (ractive) {
            ractiveValueRecoverSession(ractive, "show_overview");
            ractiveValueRecoverSession(ractive, "show_expert_plots");

            // setup the jquery ui toggle buttons
            // this can only be done here, otherwise the initial values of the toggle buttons
            // will not be correct
            /*
            do not enable jquery ui buttons atm, because the toggle option
            cannot be properly initialized with color
            $("#check_show_overview").button();
            $("#check_show_expert_plots").button();*/

            // make sure changes to the viewing settings are stored right away
            ractive.observe('show_overview', function () {
                ractiveValuePreserveSession(ractive, "show_overview");
            });
            ractive.observe('show_expert_plots', function () {
                ractiveValuePreserveSession(ractive, "show_expert_plots");
            });

            // check if an "empty" entry needs to be added to the script accordion
            if ( $('.failed_script').length > 0) {
                $("#no_failed_scripts").hide();
            }

            if ( $('.finished_script').length > 0) {
                $("#no_finished_scripts").hide();
            }

            if ( $('.skipped_script').length > 0) {
                $("#no_skipped_scripts").hide();
            }

        },
        // on teardown
        function (ractive) {
        },
        // on render
        function () {
            $("#accordion_script_files").accordion({
                heightStyle: "content"
            });
        },
        // on change
        function (ra) {
        }
    );
}

// ============================================================================
// Typesetting
// ============================================================================

/**
 * Load the Ntuple json file from a server and transfer
 * it into a HTML table
 * @param domId
 * @param jsonLoadingPath
 */
function fillNtupleTable(domId, jsonLoadingPath) {
    // move out of the static folder
    $.getJSON(`../${jsonLoadingPath}`, function (data) {
        let items = [];

        // add header
        items.push("<tr>");
        items.push("<th>tag</th>");

        // get the name of each value which is plotted
        for (let rev in data) {
            for (let fig in data[rev]) {
                let val_pair = data[rev][fig];
                items.push(`<th>${val_pair[0]}</th>`);
            }
            break;
        }

        items.push("</tr>");

        // reference first, if available
        $.each(data, function (key) {

            if (key === "reference") {
                items.push("<tr>");
                items.push(`<td>${key}</td>`);
                for (let fig in data[key]) {
                    let val_pair = data[key][fig];
                    items.push(`<td>${val_pair[1]}</td>`);
                }
                items.push("</tr>");
            }
        });

        // now the rest
        $.each(data, function (key) {
            if (key !== "reference") {
                items.push("<tr>");
                items.push(`<td>${key}</td>`);
                for (let fig in data[key]) {
                    let val_pair = data[key][fig];
                    items.push(`<td>${val_pair[1]}</td>`);
                }
                items.push("</tr>");
            }
        });

        $(`#${domId}`).after(items);
    });
}


// ============================================================================
// Small helper functions
// ============================================================================

/**
 * The package that is opened, when the validation page is opened.
 *  Currently that's just picking the page first in alphabetic order
 *  (i.e. analysis) or false if no packages are available.
 * @param packageList
 * @return {*}
 */
function getDefaultPackageName(packageList) {
    if (packageList.length === 0) {
        console.debug("getDefaultPackageName: No packages available.");
        return false;
    }

    let firstPackageName = packageList[0].name;
    if (firstPackageName !== 'undefined') {
        return firstPackageName;
    } else {
        console.debug("getDefaultPackageName: Name of first package undefined.");
        return false;
    }

}


/**
 * Returns array with the names of the selected revisions.
 * @return {Array}
 */
function getSelectedRevsList() {
    let selectedRev = [];
    $('.reference-checkbox').each(function (i, obj) {
        if (obj.checked === true) {
            selectedRev.push(obj.value)
        }
    });
    selectedRev.sort();
    return selectedRev;
}

/**
 * Returns a string representation of the array of selected revisions.
 * We need that to create folder names & queries
 * @return {string}
 */
function getSelectedRevsString() {
    let revString = "";
    let selectedRev = getSelectedRevsList();
    for (let i in selectedRev) {
        if (i > 0)
            revString += "_";
        revString += selectedRev[i];
    }
    return revString;
}

/**
 * Return the newest revision that is included in the dataset.
 * @param rev_data
 * @return {*}
 */
function getNewestRevision(rev_data) {
    let newest = null;
    // deliberately super early date
    let newestData = "2000-00-00 00:00:00";
    let revList = rev_data["revisions"];

    for (let i in revList) {
        if (revList[i]["label"] !== "reference") {
            if (revList[i]["creation_date"] > newestData) {
                newestData = revList[i]["creation_date"];
                newest = revList[i]
            }
        }
    }

    return newest
}
