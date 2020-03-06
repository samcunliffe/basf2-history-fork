# -*- coding: utf-8 -*-

"""Implements some extra utilities for doing KLM calibration with the CAF."""

import collections
import os

import basf2
from ROOT import Belle2

from caf.utils import IoV, AlgResult
from caf.utils import runs_overlapping_iov, runs_from_vector
from caf.strategies import AlgorithmStrategy
from caf.state_machines import AlgorithmMachine
from ROOT.Belle2 import KLMStripEfficiencyAlgorithm


def calibration_result_string(result):
    if (result == 0):
        res = 'successful'
    elif (result == 1):
        res = 'iteration is necessary'
    elif (result == 2):
        res = 'not enough data'
    elif (result == 3):
        res = 'failure'
    elif (result == 4):
        res = 'undefined'
    return res


class KLMStripEfficiency(AlgorithmStrategy):
    """Custom strategy for executing the KLM strip efficiency. Requires complex run merging rules.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly."""

    #: The params that you could set on the Algorithm object which this Strategy would use.
    #: Just here for documentation reasons.
    usable_params = {"apply_iov": IoV}

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help set up and execute CalibrationAlgorithm
        #: It gets setup properly in :py:func:`run`
        self.machine = AlgorithmMachine(self.algorithm)
        self.first_execution = True

    def run(self, iov, iteration, queue):
        """
        Runs the algorithm machine over the collected data and fills the results.
        """
        if not self.is_valid():
            raise StrategyError("This AlgorithmStrategy was not set up correctly!")
        #: The multiprocessing queue we use to pass back results one at a time
        self.queue = queue

        basf2.B2INFO(f"Setting up {self.__class__.__name__} strategy for {self.algorithm.name}")
        # Now add all the necessary parameters for a strategy to run
        machine_params = {}
        machine_params["database_chain"] = self.database_chain
        machine_params["dependent_databases"] = self.dependent_databases
        machine_params["output_dir"] = self.output_dir
        machine_params["output_database_dir"] = self.output_database_dir
        machine_params["input_files"] = self.input_files
        machine_params["ignored_runs"] = self.ignored_runs
        self.machine.setup_from_dict(machine_params)
        # Start moving through machine states
        basf2.B2INFO(f"Starting AlgorithmMachine of {self.algorithm.name}")
        self.algorithm.algorithm.setCalibrationStage(
            KLMStripEfficiencyAlgorithm.c_EfficiencyMeasurement)
        # This sets up the logging and database chain, plus it assigns all input files from collector jobs.
        # Note that we simply assign all input files, and use the execute(runs) to define which data to use.
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        basf2.B2INFO(f"Beginning execution of {self.algorithm.name} using strategy {self.__class__.__name__}")

        all_runs_collected = set(runs_from_vector(self.algorithm.algorithm.getRunListFromAllData()))
        # If we were given a specific IoV to calibrate we just execute all runs in that IoV at once
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected

        # Remove the ignored runs from our run list to execute
        if self.ignored_runs:
            basf2.B2INFO(f"Removing the ignored_runs from the runs to execute for {self.algorithm.name}")
            runs_to_execute.difference_update(set(self.ignored_runs))
        # Sets aren't ordered so lets go back to lists and sort
        runs_to_execute = sorted(runs_to_execute)

        apply_iov = None
        # if "apply_iov" in self.algorithm.params:
        #    apply_iov = self.algorithm.params["apply_iov"]

        # Sort runs by experiment.
        experiment_runs = collections.defaultdict(list)
        for exp_run in runs_to_execute:
            experiment_runs[exp_run.exp].append(exp_run)

        # Process experiment.
        for experiment in experiment_runs.keys():
            self.process_experiment(experiment, experiment_runs, iteration,
                                    apply_iov)

        # Whenever you have a final result that can't be changed e.g. a success you committed, or a failure you can't
        # force past or merge runs to avoid. You should send that result out.
        self.send_result(self.machine.result)

        # Check the result and commit
        if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
            # Tell the controlling process that we are done and whether the overall algorithm process managed to succeed
            self.send_final_state(self.COMPLETED)
        else:
            # Tell the controlling process that we are done and whether the overall algorithm process managed to succeed
            self.send_final_state(self.FAILED)

    def execute_over_run_list(self, run_list, iteration, apply_iov,
                              forced_calibration, calibration_stage,
                              output_file):
        """
        Execute over run list.
        """
        if not self.first_execution:
            self.machine.setup_algorithm()
        else:
            self.first_execution = False
        self.machine.algorithm.algorithm.setForcedCalibration(
            forced_calibration)
        self.machine.algorithm.algorithm.setCalibrationStage(calibration_stage)
        if (output_file is not None):
            self.machine.algorithm.algorithm.setOutputFileName(output_file)
        self.machine.execute_runs(runs=run_list, iteration=iteration,
                                  apply_iov=apply_iov)
        if (self.machine.result.result == AlgResult.ok.value) or \
           (self.machine.result.result == AlgResult.iterate.value):
            self.machine.complete()
        else:
            self.machine.fail()

    def process_experiment(self, experiment, experiment_runs, iteration,
                           apply_iov):
        """
        Process runs from experiment.
        """
        # Run lists. They have the following format: run number,
        # calibration result code, ExpRun, algorithm results,
        # merge information, payload.
        run_data = []

        # Initial run.
        for exp_run in experiment_runs[experiment]:
            self.execute_over_run_list(
                [exp_run], iteration, apply_iov, False,
                KLMStripEfficiencyAlgorithm.c_MeasurablePlaneCheck, None)
            result = self.machine.result.result
            algorithm_results = KLMStripEfficiencyAlgorithm.Results(
                self.machine.algorithm.algorithm.getResults())
            # If number of hits is 0, then KLM is excluded. Such runs
            # can be ignored safely.
            if (algorithm_results.getExtHits() > 0):
                run_data.append([exp_run.run, result, [exp_run],
                                 algorithm_results, '', None])
            result_str = calibration_result_string(result)
            basf2.B2INFO('Run %d: %s.' % (exp_run.run, result_str))

        # Sort by run number.
        run_data.sort(key=lambda x: x[0])

        # Create list of runs that do not have enough data.
        run_ranges = []
        i = 0
        while (i < len(run_data)):
            if (run_data[i][1] == 2):
                j = i
                while (run_data[j][1] == 2):
                    j += 1
                    if (j >= len(run_data)):
                        break
                run_ranges.append([i, j])
                i = j
            else:
                i += 1

        # Determine whether the runs with insufficient data can be merged to
        # the next or previous normal run.
        def can_merge(run_data, run_not_enough_data, run_normal):
            return run_data[run_not_enough_data][3].newExtHitsPlanes(
                       run_data[run_normal][3].getExtHitsPlane()) == 0

        for run_range in run_ranges:
            next_run = run_range[1]
            # To mark as 'none' at the end if there are no normal runs.
            j = run_range[0]
            i = next_run - 1
            if (next_run < len(run_data)):
                while (i >= run_range[0]):
                    if (can_merge(run_data, i, next_run)):
                        basf2.B2INFO('Run %d (not enough data) can be merged '
                                     'into the next normal run %d.' %
                                     (run_data[i][0], run_data[next_run][0]))
                        run_data[i][4] = 'next'
                    else:
                        basf2.B2INFO('Run %d (not enough data) cannot be '
                                     'merged into the next normal run %d, '
                                     'will try the previous one.' %
                                     (run_data[i][0], run_data[next_run][0]))
                        break
                    i -= 1
                if (i < run_range[0]):
                    continue
            previous_run = run_range[0] - 1
            if (previous_run >= 0):
                while (j <= i):
                    if (can_merge(run_data, j, previous_run)):
                        basf2.B2INFO('Run %d (not enough data) can be merged '
                                     'into the previous normal run %d.' %
                                     (run_data[j][0],
                                      run_data[previous_run][0]))
                        run_data[j][4] = 'previous'
                    else:
                        basf2.B2INFO('Run %d (not enough data) cannot be '
                                     'merged into the previous normal run %d.' %
                                     (run_data[j][0],
                                      run_data[previous_run][0]))
                        break
                    j += 1
                if (j > i):
                    continue
            basf2.B2INFO('A range of runs with not enough data is found '
                         'that cannot be merged into neither previous nor '
                         'next normal run: from %d to %d.' %
                         (run_data[j][0], run_data[i][0]))
            while (j <= i):
                run_data[j][4] = 'none'
                j += 1

        # Merge runs that do not have enough data. If both this and next
        # run do not have enough data, then merge the collected data.
        i = 0
        j = 0
        while (i < len(run_data) - 1):
            while ((run_data[i][1] == 2) and (run_data[i + 1][1] == 2)):
                if (run_data[i][4] != run_data[i + 1][4]):
                    break
                basf2.B2INFO('Merging run %d (not enough data) into '
                             'run %d (not enough data).' %
                             (run_data[i + 1][0], run_data[i][0]))
                run_data[i][2].extend(run_data[i + 1][2])
                del run_data[i + 1]
                self.execute_over_run_list(
                    run_data[i][2], iteration, apply_iov, False,
                    KLMStripEfficiencyAlgorithm.c_MeasurablePlaneCheck, None)
                run_data[i][1] = self.machine.result.result
                run_data[i][3] = KLMStripEfficiencyAlgorithm.Results(
                    self.machine.algorithm.algorithm.getResults())
                result_str = calibration_result_string(run_data[i][1])
                basf2.B2INFO('Run %d: %s.' % (run_data[i][0], result_str))
                if (i >= len(run_data) - 1):
                    break
            i += 1

        # Merge runs that do not have enough data into normal runs.
        def merge_runs(run_data, run_not_enough_data, run_normal, forced):
            basf2.B2INFO('Merging run %d (not enough data) into '
                         'run %d (normal).' %
                         (run_data[run_not_enough_data][0],
                          run_data[run_normal][0]))
            run_data[run_normal][2].extend(run_data[run_not_enough_data][2])
            self.execute_over_run_list(
                run_data[run_normal][2], iteration, apply_iov, forced,
                KLMStripEfficiencyAlgorithm.c_MeasurablePlaneCheck, None)
            run_data[run_normal][1] = self.machine.result.result
            run_data[run_normal][3] = KLMStripEfficiencyAlgorithm.Results(
                self.machine.algorithm.algorithm.getResults())
            result_str = calibration_result_string(run_data[run_normal][1])
            basf2.B2INFO('Run %d: %s.' % (run_data[run_normal][0], result_str))
            if (run_data[run_normal][1] != 0):
                basf2.B2FATAL('Merging run %d into run %d failed.' %
                              (run_data[run_not_enough_data][0],
                               run_data[run_normal][0]))
            del run_data[run_not_enough_data]

        i = 0
        while (i < len(run_data)):
            if (run_data[i][1] == 2):
                if (run_data[i][4] == 'next'):
                    merge_runs(run_data, i, i + 1, False)
                elif (run_data[i][4] == 'previous'):
                    merge_runs(run_data, i, i - 1, False)
                else:
                    i += 1
            else:
                i += 1
        i = 0
        while (i < len(run_data)):
            if (run_data[i][1] == 2 and run_data[i][4] == 'none'):
                new_planes_previous = -1
                new_planes_next = -1
                if (i < len(run_data) - 1):
                    new_planes_next = run_data[i][3].newExtHitsPlanes(
                        run_data[i + 1][3].getExtHitsPlane())
                    basf2.B2INFO('There are %d new active modules in run %d '
                                 'relatively to run %d.' %
                                 (new_planes_next, run_data[i][0],
                                  run_data[i + 1][0]))
                if (i > 0):
                    new_planes_previous = run_data[i][3].newExtHitsPlanes(
                        run_data[i - 1][3].getExtHitsPlane())
                    basf2.B2INFO('There are %d new active modules in run %d '
                                 'relatively to run %d.' %
                                 (new_planes_previous,
                                  run_data[i][0], run_data[i - 1][0]))
                run_for_merging = -1
                # If a forced merge of the normal run with another run from
                # a different range of runs with not enough data has already
                # been performed, then the list of active modules may change
                # and there would be 0 new modules. Consequently, the number
                # of modules is checked to be greater or equal than 0. However,
                # there is no guarantee that the same added module would be
                # calibrated normally. Thus, a forced merge is performed anyway.
                if (new_planes_previous >= 0 and new_planes_next < 0):
                    run_for_merging = i - 1
                elif (new_planes_previous < 0 and new_planes_next >= 0):
                    run_for_merging = i + 1
                elif (new_planes_previous >= 0 and new_planes_next >= 0):
                    if (new_planes_previous < new_planes_next):
                        run_for_merging = i - 1
                    else:
                        run_for_merging = i + 1
                else:
                    basf2.B2INFO('Cannot determine run for merging for run %d, '
                                 'performing its forced calibration.' %
                                 (run_data[i][0]))
                    self.execute_over_run_list(
                        run_data[i][2], iteration, apply_iov, True,
                        KLMStripEfficiencyAlgorithm.c_MeasurablePlaneCheck,
                        None)
                    run_data[i][1] = self.machine.result.result
                    run_data[i][3] = KLMStripEfficiencyAlgorithm.Results(
                        self.machine.algorithm.algorithm.getResults())
                    result_str = calibration_result_string(run_data[i][1])
                    if (run_data[i][1] != 0):
                        basf2.B2FATAL('Forced calibration of run %d failed.' %
                                      (run_data[i][0]))
                    basf2.B2INFO('Run %d: %s.' % (run_data[i][0], result_str))
                if (run_for_merging >= 0):
                    merge_runs(run_data, i, run_for_merging, True)
            else:
                i += 1

        # Stage 2: determination of maximal run ranges.
        # The set of calibrated planes should be the same for all small
        # run ranges within the large run range.
        run_ranges.clear()
        i = 0
        while (i < len(run_data)):
            j = i + 1
            while (j < len(run_data)):
                planes_differ = False
                if (run_data[j][3].newMeasuredPlanes(
                        run_data[i][3].getEfficiency()) != 0):
                    planes_differ = True
                if (run_data[i][3].newMeasuredPlanes(
                        run_data[j][3].getEfficiency()) != 0):
                    planes_differ = True
                if (planes_differ):
                    basf2.B2INFO('Run %d: the set of planes is different '
                                 'from run %d.'
                                 % (run_data[j][0], run_data[i][0]))
                    break
                else:
                    basf2.B2INFO('Run %d: the set of planes is the same '
                                 'as for run %d.'
                                 % (run_data[j][0], run_data[i][0]))
                    j = j + 1
            run_ranges.append([i, j])
            i = j

        # Stage 3: final calibration.

        # Output directory.
        if (not os.path.isdir('efficiency')):
            os.mkdir('efficiency')

        # Merge runs.
        def merge_runs_2(run_data, run_1, run_2, forced):
            basf2.B2INFO('Merging run %d into run %d.' %
                         (run_data[run_2][0], run_data[run_1][0]))
            run_data[run_1][2].extend(run_data[run_2][2])
            output_file = 'efficiency/efficiency_%d_%d.root' % \
                          (run_data[run_1][2][0].exp, run_data[run_1][2][0].run)
            self.execute_over_run_list(
                run_data[run_1][2], iteration, apply_iov, forced,
                KLMStripEfficiencyAlgorithm.c_EfficiencyMeasurement,
                output_file)
            run_data[run_1][1] = self.machine.result.result
            run_data[run_1][3] = KLMStripEfficiencyAlgorithm.Results(
                self.machine.algorithm.algorithm.getResults())
            run_data[run_1][5] = \
                self.machine.algorithm.algorithm.getPayloadValues()
            result_str = calibration_result_string(run_data[run_1][1])
            basf2.B2INFO('Run %d: %s; requested precision %f, achieved '
                         'precision %f.' %
                         (run_data[run_1][0], result_str, 0.02,
                          # run_data[run_1][3].getRequestedPrecision(),
                          run_data[run_1][3].getAchievedPrecision()))

        for run_range in run_ranges:
            i = run_range[0]
            while (i < run_range[1]):
                output_file = 'efficiency/efficiency_%d_%d.root' % \
                              (run_data[i][2][0].exp, run_data[i][2][0].run)
                # Force calibration if there are no more runs in the range.
                if (i == run_range[1] - 1):
                    forced_calibration = True
                else:
                    forced_calibration = False
                self.execute_over_run_list(
                    run_data[i][2], iteration, apply_iov, forced_calibration,
                    KLMStripEfficiencyAlgorithm.c_EfficiencyMeasurement,
                    output_file)
                run_data[i][1] = self.machine.result.result
                run_data[i][3] = KLMStripEfficiencyAlgorithm.Results(
                    self.machine.algorithm.algorithm.getResults())
                run_data[i][5] = \
                    self.machine.algorithm.algorithm.getPayloadValues()
                result_str = calibration_result_string(run_data[i][1])
                basf2.B2INFO('Run %d: %s; requested precision %f, achieved '
                             'precision %f.' %
                             (run_data[i][0], result_str, 0.02,
                              # run_data[i][3].getRequestedPrecision(),
                              run_data[i][3].getAchievedPrecision()))
                if (run_data[i][1] == 2):
                    j = i + 1
                    while (j < run_range[1]):
                        # Force calibration if there are no more runs
                        # in the range.
                        if (i == run_range[1] - 1):
                            forced_calibration = True
                        else:
                            forced_calibration = False
                        merge_runs_2(run_data, i, j, forced_calibration)
                        run_data[j][1] = -1
                        j = j + 1
                        if (run_data[i][1] == 0):
                            break
                    i = j
                else:
                    i = i + 1

        i = 0
        while (i < len(run_data)):
            if (run_data[i][1] == -1):
                del run_data[i]
            else:
                i = i + 1

        # Stage 4: write the results to the database.
        def commit_payload(run_data, run):
            basf2.B2INFO('Writing run %d.' % (run_data[run][0]))
            self.machine.algorithm.algorithm.commit(run_data[run][5])

        for i in range(0, len(run_data)):
            # Get first run again due to possible mergings.
            run_data[i][2].sort(key=lambda x: x.run)
            first_run = run_data[i][2][0].run
            # Set IOV for the current run.
            # The last run will be overwritten when writing the result.
            run_data[i][5].front().iov = \
                Belle2.IntervalOfValidity(experiment, first_run, experiment, -1)
            # If the calibration result is different, write the previous run.
            if (i > 0):
                iov = run_data[previous_run][5].front().iov
                run_data[previous_run][5].front().iov = \
                    Belle2.IntervalOfValidity(experiment, iov.getRunLow(),
                                              experiment, first_run - 1)
                commit_payload(run_data, previous_run)
                previous_run = i
            if (i == 0):
                previous_run = 0
            # Write the current run if it is the last run.
            if (i == len(run_data) - 1):
                iov = run_data[i][5].front().iov
                run_data[i][5].front().iov = \
                    Belle2.IntervalOfValidity(experiment, iov.getRunLow(),
                                              -1, -1)
                commit_payload(run_data, i)
