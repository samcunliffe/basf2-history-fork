#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import Module

from ROOT import gSystem
gSystem.Load('libframework')  # for PyStoreArray
gSystem.Load('libcdc')  # for CDCSimHit
gSystem.Load('libtracking')  # for CDCHit and so on
gSystem.Load('libgenfit2')  # for GFTrackCands

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std
from ROOT import genfit

import os
import os.path
import math

import subprocess
from datetime import datetime

from . import svgdrawing
from .svgdrawing import attributemaps


class CDCSVGDisplayModule(Module):

    """
    Personal two dimensional event display based on scalable vector graphics
    """

    def __init__(self, output_folder="/tmp", interactive=True):
        """
        Constructor method

        Parameters
        ----------
        output_folder : str
            Target folder for the output
        interactive : bool, optional
            Switch to display each event to the user and ask to continue after each event
            Defaults to True
        """

        super(CDCSVGDisplayModule, self).__init__()
        # call constructor of base class, required!

        #: Switch if the module shall show the event to the user and wait to continue or just generate the images
        self.interactive = interactive

        #: Folder the images shall be saved to
        self.output_folder = output_folder

        # List of drawing options

        # Animate the display by uncovering the drawn objects in order of their time of flight
        # This can be seen in most standard browsers. Note however that you should switch of
        # the wires in this case to reduce the rendering load.
        #: Switch to make an animated event display by means of animated SVG.
        self.animate = False

        # The following options can be used independent of the track finder
        # to view Monte Carlo information after the simulation is done

        #: Flag to use cpp
        self.use_cpp = True

        #: Flag to use python
        self.use_python = False

        #: Switch to draw the wires.
        self._draw_wires = True
        # This is wrapped by an extra property,
        # because it should always be deactivated in case the scene is animated

        #: Switch to draw the CDCHits. Default: active
        self.draw_hits = True

        #: Switch to draw the CDCHits colored by the associated CDCWireHit taken flag. Default: active
        self.draw_takenflag = False

        #: Switch to draw the interaction point. Default: active
        self.draw_interaction_point = True

        #: Switch to draw the superlayer boundaries. Default: inactive
        self.draw_superlayer_boundaries = False

        #: Switch to draw the superlayer boundaries. Default: inactive
        self.draw_cdc_walls = False

        #: Switch to draw the MCParticle::getArrayIndex property. Default: inactive
        self.draw_mcparticle_id = False

        #: Switch to draw the MCParticle::getPDGCode property. Default: inactive
        self.draw_mcparticle_pdgcodes = False

        #: Switch to draw the MCParticle::hasStatus(c_PrimaryParticle) property. Default: inactive
        self.draw_mcparticle_primary = False

        #: Switch to draw the Monte Carlo segments as generated in CDCMCTrackStore. Default: inactive
        self.draw_mcsegments = False

        #: Switch to draw the CDCSimHits with momentum information. Default: inactive
        self.draw_simhits = False

        #: Switch to draw the CDCSimHits color coded by their time of flight. Default: inactive
        self.draw_simhit_tof = False

        #: Switch to draw the CDCSimHits color coded by their getPosFlag() property. Default: inactive
        self.draw_simhit_posflag = False

        #: Switch to draw the CDCSimHits color coded by their getPDGCode() property. Default: inactive
        self.draw_simhit_pdgcode = False

        #: Switch to draw the CDCSimHits color coded by their getBackgroundTag() property.
        #: Default: inactive
        self.draw_simhit_bkgtag = False

        #: Switch to draw the CDCSimHits red for getBackgroundTag() != bg_none. Default: inactive
        self.draw_simhit_isbkg = False

        #: Switch to draw the CDCSimHits connected in the order of their getFlightTime
        #: for each Monte Carlo particle.
        #: Default: inactive
        self.draw_connect_tof = False

        #: Switch to draw the CDCSimHits color coded by their local right left passage information.
        #: Default: inactive
        self.draw_rlinfo = False

        #: Switch to draw the CDCSimHits color coded by their reassignement information
        #: to a different MCParticle.
        #: Default: inactive
        self.draw_reassigned = False

        #: Switch to draw the axial to axial segment pairs from Monte Carlo truth. Default: inactive
        self.draw_mcaxialaxialpairs = False

        #: Switch to draw the axial to stereo segment pairs from Monte Carlo truth. Default: inactive
        self.draw_axialstereopairs = False

        #: Switch to draw the segment triples from Monte Carlo truth. Default: inactive
        self.draw_mcsegmenttriples = False

        # Those are only available if the local track finder is in the module chain
        # and specific compile time flags enable to transportation of this data

        #: Switch to draw the clusters generated by the finder
        self.draw_superclusters = False

        #: Switch to draw the clusters generated by the finder
        self.draw_clusters = False

        #: Switch to draw the segments generated by the finder
        self.draw_segments_id = False

        #: Switch to draw the tangent segments generated by the finder
        self.draw_tangentsegments = False

        #: Switch to draw the segments generated by the finder colored with the Monte Carlo track id
        self.draw_segments_mctrackid = False

        #: Switch to draw the segments generated by the finder colored by the coalignment information
        #: (forward, backward, undetermined)
        self.draw_segments_fbinfo = False

        #: Switch to draw the segments generated by the finder colored by the frist in track hit id
        self.draw_segments_firstInTrackId = False

        #: Switch to draw the segments generated by the finder colored by the second in track hit id
        self.draw_segments_lastInTrackId = False

        #: Switch to draw the segments generated by the finder colored by the number of passed
        #: superlayers assoziated to the first hit in the segment
        self.draw_segments_firstNPassedSuperLayers = False

        #: Switch to draw the segments generated by the finder colored by the number of passed
        #: superlayers assoziated to the last hit in the segment
        self.draw_segments_lastNPassedSuperLayers = False

        #: Switch to draw the axial stereo segment pairs generated by the finder
        self.draw_axialstereosegmentpairs = False

        #: Switch to draw the segment triples generated by the finder
        self.draw_segmenttriples = False

        #: Switch to draw the tracks generated by the finder
        self.draw_tracks = False

        #: Switch to draw the trajectories of the tracks generated by the finder
        self.draw_track_trajectories = False

        #: Switch to draw the trajectories fitted to the segment triples generated by the finder
        self.draw_segmenttriple_trajectories = False

        #: Switch to draw the trajectories fitted to the segments generated by the finder
        self.draw_segment_trajectories = False

        # Those are only available, if any track finder is in the module chain (not tested for others than the local track finder)

        #: Draw the output Genfit trackcands
        self.draw_gftrackcands = False

        #: Draw the output Genfit tracks
        self.draw_gftracks = False

        #: Draw the output Genfit track trajectories
        self.draw_gftrack_trajectories = False

        #: Draw the trajectories stored in the output Genfit tracks
        self.draw_gftrackcand_trajectories = False

        #: Draw a red cdc hit of the rl info of the track reco hits is wrong, else a green one
        self.draw_wrong_rl_infos_in_tracks = False

        #: Draw a red cdc hit of the rl info of the segment reco hits is wrong, else a green one
        self.draw_wrong_rl_infos_in_segments = False

        #: Name of the CDC Hits store array
        self.cdc_hits_store_array_name = "CDCHits"

        #: Name of the genfit track cand store array
        self.track_cands_store_array_name = "TrackCands"

        #: Name of the CDC Wire Hit Clusters
        self.cdc_wire_hit_cluster_store_obj_name = "CDCWireHitClusterVector"

        #: Name of the CDC Reco Segment Vector
        self.cdc_reco_segment_vector_store_obj_name = 'CDCRecoSegment2DVector'

        #: Current file's number (used for making output filename)
        self.file_number = 1

        #: Filename prefix
        self.filename_prefix = "CDCDisplay"

        #: Use time instead of prefix in filename
        self.use_time_in_filename = True or False

    @property
    def drawoptions(self):
        """
        Property that collects the various names of the draw options to a list
        that are not related to the CDC cellular automaton track finder.
        @return list of strings naming the different switches that can be activated.
        """

        result = [
            'animate',
            'draw_superlayer_boundaries',
            'draw_cdc_walls',
            'draw_interaction_point',
            'draw_mcparticle_id',
            'draw_mcparticle_pdgcodes',
            'draw_mcparticle_primary',
            'draw_mcsegments',
            'draw_simhits',
            'draw_simhit_tof',
            'draw_simhit_posflag',
            'draw_simhit_pdgcode',
            'draw_simhit_bkgtag',
            'draw_simhit_isbkg',
            'draw_connect_tof',
            'draw_rlinfo',
            'draw_reassigned',
            'draw_gftrackcands',
            'draw_gftracks',
            'draw_gftrackcand_trajectories',
            'draw_gftrack_trajectories',
            # Specialised options to be used in the CDC local tracking context
            # obtain them from the all_drawoptions property
            # 'draw_segments_id',
            # 'draw_tangentsegments',
            # 'draw_segments_mctrackid',
            # 'draw_takenflag',
            # 'draw_mcaxialaxialpairs',
            # 'draw_axialstereopairs',
            # 'draw_mcsegmenttriples',
            # 'draw_superclusters',
            # 'draw_clusters',
            # 'draw_segments_fbinfo',
            # 'draw_segments_firstInTrackId',
            # 'draw_segments_lastInTrackId',
            # 'draw_segments_firstNPassedSuperLayers',
            # 'draw_segments_lastNPassedSuperLayers',
            # 'draw_axialstereosegmentpairs',
            # 'draw_segmenttriples',
            # 'draw_tracks',
            # 'draw_segmenttriple_trajectories',
            # 'draw_segment_trajectories',
        ]

        for name in result:
            if not hasattr(self, name):
                raise NameError('%s is not a valid draw option. Fix the misspelling.'
                                % name)

        return result

    @property
    def all_drawoptions(self):
        """
        Property that collects the all names of the draw options to a list.
        Note that some draw options only make sense after running the CDC
        cellular automaton track finder.
        @return list of strings naming the different switches that can be activated.
        """
        result = self.drawoptions

        # Add all attributes that start with draw
        draw_options = [option for option in self.__dict__
                        if option.startswith('draw_')]
        _draw_options = [option[1:] for option in self.__dict__
                         if option.startswith('_draw_')]

        result.extend(draw_options)
        result.extend(_draw_options)

        return set(result)

    @property
    def draw_wires(self):
        """
        Getter for the draw option that indicates if all wires shall be drawn.
        Since this has some performance impact in animated events the wires
        are prevented from being drawn in this case.
        """

        return self._draw_wires and not self.animate

    @draw_wires.setter
    def draw_wires(self, draw_wires):
        """
        Setter for the option to draw all wires.
        """

        self._draw_wires = draw_wires

    def initialize(self):
        """
        Initialisation method of the module.
        Creates the output folder if it is not present yet.
        """

        output_folder = self.output_folder
        if not os.path.exists(output_folder):
            print("CDCSVGDisplay.__init__ : Output folder '", output_folder,
                  "' does not exist.")
            answer = input('Create it? (y or n)')
            if answer == 'y':
                os.makedirs(output_folder)

        # Make sure at least one backend is available
        if not self.use_cpp and not self.use_python:
            self.use_cpp = True

        if self.use_cpp:
            cppplotter = Belle2.TrackFindingCDC.CDCSVGPlotter(self.animate)
        if self.use_python:
            plotter = svgdrawing.CDCSVGPlotter(animate=self.animate)

        # ######### CDCWires ##########
        # Draw wires from cdcwire objects
        # Now prefered way of ploting the wires
        if self.draw_wires:
            theCDCWireTopology = \
                Belle2.TrackFindingCDC.CDCWireTopology.getInstance()

            if self.use_cpp:
                cppplotter.drawWires(theCDCWireTopology)
            if self.use_python:
                plotter.draw(theCDCWireTopology)

        if self.use_cpp:
            self.prefilled_cppplotter = cppplotter
        if self.use_python:
            self.prefilled_plotter = plotter

    def beginRun(self):
        """
        Begin run method of the module. Empty here.
        """

        pass

    def event(self):
        """
        Event method of the module. Draws the event into a new svg file.
        """

        # Clone the plotter that contains the wires already
        if self.use_cpp:
            cppplotter = self.prefilled_cppplotter.clone()
        if self.use_python:
            plotter = self.prefilled_plotter.clone()

        # if self.draw_wires:
        #    theCDCWireTopology = \
        #        Belle2.TrackFindingCDC.CDCWireTopology.getInstance()
        #    cppplotter.draw(theCDCWireTopology)

        # Plotter instance receiving drawable tracking objects.
        # self.plotter = plotter

        # Construct additional information from basic Monte Carlo data, if it is available from the DataStore
        # Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()

        # ######### CDCHits ##########
        # Draw the raw CDCHits
        if self.draw_hits:
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'ZeroDriftLengthColorMap', 'ZeroDriftLengthStrokeWidthMap')
            if self.use_python:
                styleDict = {'stroke': attributemaps.ZeroDriftLengthColorMap(),
                             'stroke-width': attributemaps.ZeroDriftLengthStrokeWidthMap()}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # Draw the CDCHits colored by the taken flag from the CDCWireHit.
        if self.draw_takenflag:
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'TakenFlagColorMap', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.TakenFlagColorMap(), }
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # Draw mcparticle id
        if self.draw_mcparticle_id:
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'MCParticleColorMap', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.MCParticleColorMap()}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # Draw the in track segment id
        if self.draw_mcsegments:
            Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'MCSegmentIdColorMap', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.MCSegmentIdColorMap()}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # Draw monte carlo pdg codes
        if self.draw_mcparticle_pdgcodes:
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'MCPDGCodeColorMap', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.MCPDGCodeColorMap()}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # Draw monte carlo pdg codes
        if self.draw_mcparticle_primary:
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'MCPrimaryColorMap', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.MCPrimaryColorMap()}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # Draw SimHits
        if self.draw_simhits:
            if self.use_cpp:
                cppplotter.drawCDCSimHits(self.cdc_hits_store_array_name, '', '.2')
            if self.use_python:
                hit_storearray = Belle2.PyStoreArray(self.cdc_hits_store_array_name)
                if hit_storearray:
                    simHits_related_to_hits = [hit.getRelated('CDCSimHits')
                                               for hit in hit_storearray]
                    styleDict = {'stroke-width': '0.2'}
                    plotter.draw_iterable(simHits_related_to_hits, **styleDict)

        # Draw RL MC info
        if self.draw_simhit_posflag:
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'PosFlagColorMap', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.PosFlagColorMap()}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # Draw local RL info
        if self.draw_rlinfo:
            Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'RLColorMap', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.RLColorMap()}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # Draw tof info
        if self.draw_simhit_tof:
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'TOFColorMap', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.TOFColorMap()}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # Draw pdg code of simhits
        if self.draw_simhit_pdgcode:
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, "SimHitPDGCodeColorMap", "")
            if self.use_python:
                def color_map(iHit, hit):
                    simHit = hit.getRelated('CDCSimHits')
                    pdgCode = simHit.getPDGCode()
                    color = \
                        attributemaps.MCPDGCodeColorMap.color_by_pdgcode.get(pdgCode,
                                                                             'orange')
                    return color

                styleDict = {'stroke': color_map}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # Draw background tag of related simhits
        if self.draw_simhit_bkgtag:
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'BackgroundTagColorMap', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.BackgroundTagColorMap()}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # Draw background tag != bg_none of related simhits
        if self.draw_simhit_isbkg:
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'SimHitIsBkgColorMap', '')
            if self.use_python:
                def color_map(iHit, hit):
                    simHit = hit.getRelated('CDCSimHits')
                    bkgTag = simHit.getBackgroundTag()
                    color = ('gray' if bkgTag else 'red')
                    return color

                styleDict = {'stroke': color_map}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        if self.draw_connect_tof:
            if self.use_cpp:
                cppplotter.drawCDCSimHitsConnectByToF(self.cdc_hits_store_array_name, "black", ".2")
            if self.use_python:
                cdchits_storearray = Belle2.PyStoreArray(self.cdc_hits_store_array_name)
                if cdchits_storearray:
                    simhits_related_to_cdchit = [cdchit.getRelated('CDCSimHits')
                                                 for cdchit in cdchits_storearray]

                    # group them by their mcparticle id
                    simhits_by_mcparticle = {}
                    for simhit in simhits_related_to_cdchit:
                        mcparticle = simhit.getRelated('MCParticles')
                        if not mcparticle == None:  # noqa
                            mcTrackId = mcparticle.getArrayIndex()
                            simhits_by_mcparticle.setdefault(mcTrackId, [])
                            simhits_by_mcparticle[mcTrackId].append(simhit)

                    for simhits_for_mcparticle in list(simhits_by_mcparticle.values()):
                        simhits_for_mcparticle.sort(key=lambda simhit:
                                                    simhit.getFlightTime())

                        nSimHits = len(simhits_for_mcparticle)
                        for iSimHit in range(nSimHits - 1):
                            fromSimHit = simhits_for_mcparticle[iSimHit]
                            toSimHit = simhits_for_mcparticle[iSimHit + 1]

                            styleDict = {'stroke-width': '0.2', "stroke": "black"}

                            fromHit = fromSimHit.getRelated(self.cdc_hits_store_array_name)
                            toHit = toSimHit.getRelated(self.cdc_hits_store_array_name)

                            fromWireHit = Belle2.TrackFindingCDC.CDCWireHit(fromHit)
                            toWireHit = Belle2.TrackFindingCDC.CDCWireHit(toHit)

                            fromRLWireHit = Belle2.TrackFindingCDC.CDCRLWireHit(fromWireHit, 0)
                            toRLWireHit = Belle2.TrackFindingCDC.CDCRLWireHit(toWireHit, 0)

                            fromDisplacement = Belle2.TrackFindingCDC.Vector3D(fromSimHit.getPosTrack() - fromSimHit.getPosWire())
                            toDisplacement = Belle2.TrackFindingCDC.Vector3D(toSimHit.getPosTrack() - toSimHit.getPosWire())

                            fromRecoHit2D = Belle2.TrackFindingCDC.CDCRecoHit2D(fromRLWireHit, fromDisplacement.xy())
                            toRecoHit2D = Belle2.TrackFindingCDC.CDCRecoHit2D(toRLWireHit, toDisplacement.xy())

                            tangent = Belle2.TrackFindingCDC.CDCTangent(fromRecoHit2D, toRecoHit2D)
                            plotter.draw(tangent, **styleDict)

        # Draw the reassignment information of hits
        if self.draw_reassigned:
            if self.use_cpp:
                cppplotter.drawCDCHits(self.cdc_hits_store_array_name, 'ReassignedSecondaryMap', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.ReassignedSecondaryMap()}
                plotter.draw_storearray(self.cdc_hits_store_array_name, **styleDict)

        # CDCWireHitClusters
        # Draw superclusters
        if self.draw_superclusters:
            if self.use_cpp:
                cppplotter.drawCDCWireHitClusters('CDCWireHitSuperClusterVector', 'ListColors', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.listColors}
                plotter.draw_storevector('CDCWireHitSuperClusterVector', **styleDict)

        # Draw clusters
        if self.draw_clusters:
            if self.use_cpp:
                cppplotter.drawCDCWireHitClusters(self.cdc_wire_hit_cluster_store_obj_name, 'ListColors', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.listColors}
                plotter.draw_storevector(self.cdc_wire_hit_cluster_store_obj_name, **styleDict)

        # ######### CDCRecoSegments2D ##########
        # Draw Segments

        if self.draw_segments_id:
            if self.use_cpp:
                cppplotter.drawCDCRecoSegments(self.cdc_reco_segment_vector_store_obj_name, "ListColors", "")
            if self.use_python:
                styleDict = {'stroke': attributemaps.listColors}
                plotter.draw_storevector(self.cdc_reco_segment_vector_store_obj_name, **styleDict)

        if self.draw_segments_mctrackid:
            Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance().fill()
            if self.use_cpp:
                cppplotter.drawCDCRecoSegments(self.cdc_reco_segment_vector_store_obj_name, "SegmentMCTrackIdColorMap", "")
            if self.use_python:
                styleDict = {'stroke': attributemaps.SegmentMCTrackIdColorMap()}
                plotter.draw_storevector(self.cdc_reco_segment_vector_store_obj_name, **styleDict)

        if self.draw_segments_fbinfo:
            if self.use_cpp:
                cppplotter.drawCDCRecoSegments(self.cdc_reco_segment_vector_store_obj_name, "SegmentFBInfoColorMap", "")
            if self.use_python:
                styleDict = {'stroke': attributemaps.SegmentFBInfoColorMap()}
                plotter.draw_storevector(self.cdc_reco_segment_vector_store_obj_name, **styleDict)

        if self.draw_segments_firstInTrackId:
            if self.use_cpp:
                cppplotter.drawCDCRecoSegments(self.cdc_reco_segment_vector_store_obj_name,
                                               "SegmentFirstInTrackIdColorMap", "")
            if self.use_python:
                styleDict = \
                    {'stroke': attributemaps.SegmentFirstInTrackIdColorMap()}
                plotter.draw_storevector(self.cdc_reco_segment_vector_store_obj_name, **styleDict)

        if self.draw_segments_lastInTrackId:
            if self.use_cpp:
                cppplotter.drawCDCRecoSegments(self.cdc_reco_segment_vector_store_obj_name, "SegmentLastInTrackIdColorMap", "")
            if self.use_python:
                styleDict = \
                    {'stroke': attributemaps.SegmentLastInTrackIdColorMap()}
                plotter.draw_storevector(self.cdc_reco_segment_vector_store_obj_name, **styleDict)

        if self.draw_segments_firstNPassedSuperLayers:
            if self.use_cpp:
                cppplotter.drawCDCRecoSegments(
                    self.cdc_reco_segment_vector_store_obj_name,
                    "SegmentFirstNPassedSuperLayersColorMap",
                    "")
            if self.use_python:
                styleDict = \
                    {'stroke': attributemaps.SegmentFirstNPassedSuperLayersColorMap()}
                plotter.draw_storevector(self.cdc_reco_segment_vector_store_obj_name, **styleDict)

        if self.draw_segments_lastNPassedSuperLayers:
            if self.use_cpp:
                cppplotter.drawCDCRecoSegments(
                    self.cdc_reco_segment_vector_store_obj_name,
                    "SegmentLastNPassedSuperLayersColorMap",
                    "")
            if self.use_python:
                styleDict = \
                    {'stroke': attributemaps.SegmentLastNPassedSuperLayersColorMap()}
                plotter.draw_storevector(self.cdc_reco_segment_vector_store_obj_name, **styleDict)

        # Mimic axial to axial pair selection
        if self.draw_mcaxialaxialpairs:
            if self.use_cpp:
                cppplotter.drawMCAxialAxialPairs(self.cdc_reco_segment_vector_store_obj_name, "black", '')
            if self.use_python:
                segment_storevector = Belle2.PyStoreObj(self.cdc_reco_segment_vector_store_obj_name)
                if segment_storevector:
                    segments = segment_storevector.obj().unwrap()
                    axial_segments = [segment for segment in segments
                                      if segment.getStereoType() == 0]

                    mc_axial_segment_pair_segment_filter = \
                        Belle2.TrackFindingCDC.MCAxialSegmentPairFilter()
                    axial_segment_pair_relations = \
                        (Belle2.TrackFindingCDC.CDCAxialSegmentPair(startSegment, endSegment)
                         for startSegment in axial_segments
                         for endSegment in axial_segments)

                    def is_good_pair(pair):
                        weight = \
                            mc_axial_segment_pair_segment_filter.isGoodAxialSegmentPair(pair)
                        return weight == weight  # not nan

                    good_axial_segment_pair_relations = [pair for pair in
                                                         axial_segment_pair_relations if is_good_pair(pair)]
                    styleDict = {"stroke": "black"}
                    plotter.draw_iterable(good_axial_segment_pair_relations, **styleDict)

        if self.draw_axialstereopairs:
            if self.use_cpp:
                cppplotter.drawAxialStereoPairs('CDCSegmentPairs', 'black', '')
            if self.use_python:
                styleDict = {"stroke": "black"}
                plotter.draw_storearray('CDCSegmentPairs', **styleDict)

        if self.draw_mcsegmenttriples:
            if self.use_cpp:
                cppplotter.drawMCSegmentTriples(self.cdc_reco_segment_vector_store_obj_name, '', '')
            if self.use_python:
                segment_storevector = Belle2.PyStoreObj(self.cdc_reco_segment_vector_store_obj_name)
                if segment_storevector:
                    segments = segment_storevector.obj().unwrap()
                    axial_segments = [segment for segment in segments
                                      if segment.getStereoType() == 0]

                    stereo_segments = [segment for segment in segments
                                       if segment.getStereoType() != 0]

                    # Misuse this a bit but still does what we want
                    mc_axial_segment_pair_segment_filter = \
                        Belle2.TrackFindingCDC.MCAxialSegmentPairFilter()
                    mc_segment_lookup = \
                        Belle2.TrackFindingCDC.CDCMCSegmentLookUp.getInstance()

                    segment_triples = \
                        (Belle2.TrackFindingCDC.CDCSegmentTriple(startSegment,
                                                                 middleSegment, endSegment) for startSegment in
                         axial_segments for middleSegment in stereo_segments
                         for endSegment in axial_segments)

                    def is_good_triple(triple):
                        start = triple.getStartSegment()
                        middle = triple.getMiddleSegment()
                        end = triple.getEndSegment()

                        pairWeight = \
                            mc_axial_segment_pair_segment_filter.isGoodAxialSegmentPair(triple)

                        if not pairWeight == pairWeight:
                            return False

                        startToMiddleFBInfo = \
                            mc_segment_lookup.areAlignedInMCTrack(start, middle)
                        if abs(startToMiddleFBInfo) > 1:
                            return False

                        middleToEndFBInfo = \
                            mc_segment_lookup.areAlignedInMCTrack(middle, end)
                        if abs(middleToEndFBInfo) > 1:
                            return False

                        if startToMiddleFBInfo == middleToEndFBInfo:
                            return True
                        else:
                            return False

                    good_segment_triples = [triple for triple in segment_triples
                                            if is_good_triple(triple)]

                    styleDict = {"stroke": "black"}
                    plotter.draw_iterable(good_segment_triples, **styleDict)

        # Draw Tangent segments
        if self.draw_tangentsegments:
            if self.use_cpp:
                print('No CPP-function defined')
            if self.use_python:
                styleDict = {'stroke-width': '0.2'}
                plotter.draw_storearray('CDCTangentSegments', **styleDict)

        # Draw axial stereo segment pairs
        if self.draw_axialstereosegmentpairs:
            if self.use_cpp:
                cppplotter.drawAxialStereoSegmentPairs('CDCSegmentPairVector', '', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.listColors}
                plotter.draw_storevector('CDCSegmentPairVector', **styleDict)

        # Draw segment triples
        if self.draw_segmenttriples:
            if self.use_cpp:
                print('No CPP-function defined')
            if self.use_python:
                styleDict = {'stroke': attributemaps.listColors}
                plotter.draw_storearray('CDCSegmentTriples', **styleDict)

        # Draw Tracks
        if self.draw_tracks:
            if self.use_cpp:
                cppplotter.drawCDCTracks('CDCTrackVector', 'ListColors', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.listColors}
                plotter.draw_storevector('CDCTrackVector', **styleDict)

        # Wrong RL Info
        if self.draw_wrong_rl_infos_in_tracks:
            if self.use_cpp:
                print('No CPP-function defined')
            if self.use_python:
                styleDict = {'stroke': attributemaps.WrongRLColorMap()}

                pystoreobj = Belle2.PyStoreObj('CDCTrackVector')

                if pystoreobj:
                    # Wrapper around std::vector like
                    wrapped_vector = pystoreobj.obj()
                    vector = wrapped_vector.get()

                    for track in vector:
                        plotter.draw_iterable(list(track.items()), **styleDict)

        if self.draw_wrong_rl_infos_in_segments:
            if self.use_cpp:
                print('No CPP-function defined')
            if self.use_python:
                styleDict = {'stroke': attributemaps.WrongRLColorMap()}

                pystoreobj = Belle2.PyStoreObj(self.cdc_reco_segment_vector_store_obj_name)

                if pystoreobj:
                    # Wrapper around std::vector like
                    wrapped_vector = pystoreobj.obj()
                    vector = wrapped_vector.get()

                    for track in vector:
                        plotter.draw_iterable(list(track.items()), **styleDict)

        # Draw Track Trajectories
        if self.draw_track_trajectories:
            if self.use_cpp:
                cppplotter.drawTrackTrajectories("CDCTrackVector",
                                                 "ListColors",
                                                 "")
            if self.use_python:
                styleDict = {'stroke': attributemaps.listColors}
                track_storevector = Belle2.PyStoreObj('CDCTrackVector')
                if track_storevector:
                    tracks = track_storevector.obj().unwrap()
                    iterTrajectories = (cdcTrack.getStartTrajectory3D().getTrajectory2D()
                                        for cdcTrack in tracks)
                    plotter.draw_iterable(iterTrajectories, **styleDict)

        # Draw the genfit track candidates
        if self.draw_gftrackcands:
            if self.use_cpp:
                cppplotter.drawGFTrackCands(self.track_cands_store_array_name, 'ListColors', '')
            if self.use_python:
                styleDict = {'stroke': attributemaps.listColors}
                plotter.draw_storearray(self.track_cands_store_array_name, **styleDict)

        # Draw the genfit track
        if self.draw_gftracks:
            cppplotter.drawGFTracks('GF2Tracks', '', '')

        # Draw interaction point
        if self.draw_interaction_point:
            if self.use_cpp:
                cppplotter.drawInteractionPoint()
            if self.use_python:
                plotter.draw_interaction_point()

        # Draw the superlayer boundaries
        if self.draw_superlayer_boundaries:
            if self.use_cpp:
                print('No CPP-function defined')
            if self.use_python:
                plotter.draw_superlayer_boundaries()

        # Draw the outer and inner wall of the cdc.
        if self.draw_cdc_walls:
            if self.use_cpp:
                cppplotter.drawOuterCDCWall('black')
                cppplotter.drawInnerCDCWall('black')
            if self.use_python:
                styleDict = {'stroke': 'black'}
                plotter.draw_outer_cdc_wall(**styleDict)
                plotter.draw_inner_cdc_wall(**styleDict)

        # Draw segment triples fits
        if self.draw_segmenttriple_trajectories:
            if self.use_cpp:
                print('No CPP-function defined')
            if self.use_python:
                segmentTriple_storearray = Belle2.PyStoreArray('CDCSegmentTriples')
                if segmentTriple_storearray:
                    iterSegmentTriples = iter(segmentTriple_storearray)
                    iterTrajectories = (segmentTriple.getTrajectory2D()
                                        for segmentTriple in iterSegmentTriples)
                    plotter.draw_iterable(iterTrajectories)

        # Draw the fits to the segments
        if self.draw_segment_trajectories:
            if self.use_cpp:
                cppplotter.drawSegmentTrajectories(self.cdc_reco_segment_vector_store_obj_name,
                                                   "ListColors",
                                                   "")
            if self.use_python:
                segment_storevector = Belle2.PyStoreObj(self.cdc_reco_segment_vector_store_obj_name)
                if segment_storevector:
                    segments = segment_storevector.obj().unwrap()

                    iterTrajectories = (segment.getTrajectory2D() for segment in
                                        segments)
                    plotter.draw_iterable(iterTrajectories)

        # Draw the trajectories of the genfit track candidates
        if self.draw_gftrackcand_trajectories:
            if self.use_cpp:
                cppplotter.drawGFTrackCandTrajectories(self.track_cands_store_array_name, '', '')
            if self.use_python:
                gftrackcand_storearray = Belle2.PyStoreArray(self.track_cands_store_array_name)
                if gftrackcand_storearray:
                    def color_map(iTrajectory, trajectory):
                        # return "black"
                        return attributemaps.listColors[iTrajectory
                                                        % len(attributemaps.listColors)]

                    styleDict = {'stroke': color_map}

                    trajectories = []
                    for gftrackcand in gftrackcand_storearray:
                        tMomentum = gftrackcand.getMomSeed()
                        charge = gftrackcand.getChargeSeed()
                        tPosition = gftrackcand.getPosSeed()

                        momentum = Belle2.TrackFindingCDC.Vector2D(tMomentum.X(),
                                                                   tMomentum.Y())
                        position = Belle2.TrackFindingCDC.Vector2D(tPosition.X(),
                                                                   tPosition.Y())

                        trajectory = \
                            Belle2.TrackFindingCDC.CDCTrajectory2D(position,
                                                                   momentum, charge)
                        trajectories.append(trajectory)

                    plotter.draw_iterable(trajectories, **styleDict)

        # Draw the genfit track-trajectories
        if self.draw_gftrack_trajectories:
            cppplotter.drawGFTrackTrajectories('GF2Tracks', '', '')

        fileName = self.new_output_filename()
        cppfileName = self.new_output_filename()

        if self.use_cpp:
            cppplotter.saveSVGFile(cppfileName)
        if self.use_python:
            plotter.saveSVGFile(fileName)

        if self.interactive:
            if self.use_python:
                print(" Use the 'display' command to show the svg file", fileName,
                      'generated for the last event')
            if self.use_cpp:
                print(" Use the 'display' command to show the svg file", cppfileName,
                      'generated for the last event with cpp')

            # 'display' is part of the ImageMagic package commonly installed in linux
            if self.use_python:
                procDisplay = subprocess.Popen(['eog', fileName])
            if self.use_cpp:
                procDisplay = subprocess.Popen(['eog', cppfileName])
            # procDisplay = subprocess.Popen(['display','-background','white',
            # '-flatten',fileName])
            # procConverter = subprocess.Popen(['rsvg', root + '.svg', root + '.png'])
            input('Hit enter for next event')

        self.file_number += 1

    def endRun(self):
        """
        endRun methode of the module. Empty here.
        """

        pass

    def terminate(self):
        """
        teminate methode of the module. Empty here.
        """

        pass

    def new_output_basename(self):
        """
        Generates a new unique name for the current event without the folder prefix
        """

        if self.use_time_in_filename:
            output_basename = datetime.now().isoformat() + '.svg'
        else:
            output_basename = self.filename_prefix + str(self.file_number).zfill(4) + '.svg'
        return output_basename

    def new_output_filename(self):
        """
        Generates a new unique name for the current event with the folder prefix
        """

        return os.path.join(self.output_folder, self.new_output_basename())
