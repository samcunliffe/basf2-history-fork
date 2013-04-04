#ifndef HOUGH3DUTILITY_H
#include <vector>
#include <TVectorD.h>
#define HOUGH3DUTILITY_H


class Hough3DFinder {
  public: 
    Hough3DFinder(void);
    Hough3DFinder(int mode);
    ~Hough3DFinder(void);
    // Member functions.
    void setMode(int mode);
    int getMode(void);
    // Geometry variables. [rr0, rr1, rr2, rr3, anglest0, anglest1, anglest2, anglest3, ztostraw0, ztostraw1, ztostraw2, ztostraw3]
    // Init variables. [cotStart, cotEnd, z0Start, z0End, nCotSteps, nZ0Steps]
    void initialize(TVectorD &geometryVariables ,std::vector<float > & initVariables);
    void destruct(void);
    // Track variables. [charge, rho, phi0]
    // Stereo TS candidates[layer][TS ID]
    void runFinder(std::vector<double> &trackVariables, std::vector<std::vector<double> > &stTSs);
    // Init variables. [cotStart, cotEnd, z0Start, z0Ent, nCotSteps, nZ0Steps]
    void initVersion1(std::vector<float > & initVariables);
    void initVersion2(std::vector<float > & initVariables);
    void initVersion3(std::vector<float > & initVariables);
    void setInputFileName(std::string inputFileName);
    void destVersion1(void);
    void destVersion2(void);
    void destVersion3(void);
    void runFinderVersion1(std::vector<double> &trackVariables, std::vector<std::vector<double> > &stTSs, std::vector<double> &tsArcS, std::vector<std::vector<double> > &tsZ);
    void runFinderVersion2(std::vector<double> &trackVariables, std::vector<std::vector<double> > &stTSs);
    void runFinderVersion3(std::vector<double> &trackVariables, std::vector<std::vector<double> > &stTSs);
    void getValues(const std::string& input, std::vector<double> &result);
    void getHoughMeshLayer(bool ***& houghMeshLayer);
    // Members.
    int m_mode;
    int m_nWires[4];
    double m_rr[4];
    double m_ztostraw[4];
    double m_anglest[4];
    double m_Trg_PI;
    // Hough Variables.
    double m_cotStart;
    double m_cotEnd;
    double m_z0Start;
    double m_z0End;
    int m_nCotSteps;
    int m_nZ0Steps;
    double m_cotStepSize;
    double m_z0StepSize;
    float ***m_houghMeshLayerDiff;
    bool ***m_houghMeshLayer;
    int **m_houghMesh;
    float **m_houghMeshDiff;
    bool ** m_hitMap;
    // GeoFinder Variables.
    std::vector< std::vector< int> > *m_geoCandidatesIndex;
    std::vector< std::vector< double> > *m_geoCandidatesPhi;
    std::vector< std::vector< double> > *m_geoCandidatesDiffStWires;
    double m_stAxPhi[4];
    // Finder results.
    double m_bestCot;
    double m_bestZ0;
    double m_houghMax;
    double m_minDiffHough;
    double m_foundZ[4];
    double m_foundPhiSt[4];
    int m_bestTSIndex[4];
    double m_bestTS[4];
    // Version3 (GeoFinder Integer space)
    std::string m_inputFileName;
    // [rho, phi0, sign]
    std::vector<double> m_FPGAInput;
    // [arcCos0, arcCos1, arcCos2, arcCos3, 
    //  bestTSIndex0, bestTSIndex1, bestTSIndex2, bestTSIndex3]
    std::vector<double> m_FPGAOutput;
    // Find min and max values
    double m_findRhoMax, m_findRhoMin;
    double m_findRhoIntMax, m_findRhoIntMin;
    double m_findPhi0Max, m_findPhi0Min;
    double m_findPhi0IntMax, m_findPhi0IntMin;
    double m_findArcCosMax, m_findArcCosMin;
    double m_findArcCosIntMax, m_findArcCosIntMin;
    double m_findPhiZMax, m_findPhiZMin;
    double m_findPhiZIntMax, m_findPhiZIntMin;
    // Integer space
    double m_rhoMax, m_rhoMin;
    int m_rhoBit;
    double m_phi0Max, m_phi0Min;
    int m_phi0Bit;
    int m_stAxWireFactor;
    // FPGA LUTs
    bool m_LUT;
    int** m_arcCosLUT;
    int** m_wireConvertLUT;

};

#ifdef __CINT__
#include "../src/Hough3DUtility.cc"
#endif

#endif
