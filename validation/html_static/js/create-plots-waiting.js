/**
 * This function installs a timer with wait_time ( in ms ) which
 * will check for the plot creation progress on the server
 * To identify a specific plotting request, the progress_key
 * handed out by the server must be passed here
 * The joined_revisions parameter is needed ot reload the
 * correct combination of revisions.
 * This method will either start a new time, if the plots are
 * not complete yet or will hide the wait dialog if the plotting
 * is complete.
 * @param revString
 * @param joinedRevisions
 * @param progressKey
 * @param waitTime
 * @param revData
 */
function installPlottingProgress(revString, joinedRevisions, progressKey, waitTime, revData) {

    // query every second
    let defaultWaitTime = 1000;

    setTimeout(function () {

        // ajax request to check for change
        $.ajax({
            url: "../check_comparison_status",
            type: "POST",
            timeout: 1600,
            contentType: "application/json",
            dataType: "json",
            data: JSON.stringify({"input": progressKey})
        })
            .success(function (ajaxResult) {
                if (!ajaxResult) {
                    // no status yet, keep on querying
                    installPlottingProgress(revString, joinedRevisions, progressKey, defaultWaitTime, revData);
                } else {
                    // is it only a status message or a message with detailed information
                    // on the current progress ?
                    if (ajaxResult["status"] === "complete") {
                        // is the plotting complete and the overlay can be hidden again ?
                        //loadcontent(joined_revisions, true);
                        //alert("complete !!");
                        // load the newly generated plots

                        // trigger complete reload !
                        // todo: the revision selection seems to be kept, why ?
                        //loadRevisions( rev_string, joined_revisions );
                        console.log("Plot creation for revisions " + revString + " complete");
                        // fixme: revList unknown??
                        setupRactiveFromRevision(revData, revString, revList);
                        $("#outer").hide();
                    } else {
                        // detailed progress message

                        // extract information from the json dict
                        let currentPackage = parseInt(ajaxResult["current_package"]);
                        let totalPackage = parseInt(ajaxResult["total_package"]);
                        let packageName = ajaxResult["package_name"];

                        // display infos on the waiting dialog
                        $("#plot_creation_progress").text("Processing " + currentPackage + " of " + totalPackage + " packages");
                        $("#plot_creation_package").text("Current Package: " + packageName);

                        // re-install the timer to check back on the progress
                        installPlottingProgress(revString, joinedRevisions, progressKey, defaultWaitTime, revData);
                    }
                }

            });
    }, waitTime);
}


/**
 * This function gets called from setupRactiveFromRevision, if we see that we
 * need to generate new plots.
 * @param revString
 * @param joinedRevisions
 * @param progressKey
 * @param revData
 */
function beginCreatePlotWait(revString, joinedRevisions, progressKey, revData) {

    // reset status display from previous execution
    $("#plot_creation_progress").text("");
    $("#plot_creation_package").text("");

    // Show the layer div that indicates that new plots are being created
    $("#outer").show();

    // check for a change already very quick, if the plots are already available
    installPlottingProgress(revString, joinedRevisions, progressKey, 0, revData)
}
