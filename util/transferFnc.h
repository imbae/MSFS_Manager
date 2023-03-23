/*
 *  transferFnc.h
 *
 *  Created on: 2023. 03. 19.
 *  Author: JSH
 */

#ifndef _TRANSFERFNC_H_
#define _TRANSFERFNC_H_

#include <iostream>
#include "../common.h"

using namespace std;

class transferFnc
{
public:
    transferFnc(void);

    ~transferFnc(void);

    typedef enum { eLPF_1stOrder, eLPF_2ndOrder, eHPF_1stOrder, eHPF_2ndOrder, eIntegrator, eFcnNum } etFilterType;

    bool                        Update(void);

    void                        MakeFilterTransferFunction(etFilterType type);

    void                        SetCutoffFrequency(double in)                               { dCutOffFrequency = in; }
    
    void                        SetDampingRatio(double in)                                  { dDampingRatio = in; }

    void                        SetInputData(double in)                                     { dInput = in; }

    double                      GetOutputData(void)                                         { return dOutput; }

private:
    /* load variables       */

    /* input variables      */
    double                      dDampingRatio;                                              /* none         -                                   */
    double                      dCutOffFrequency;                                           /* 1/sec        -                                   */
    double                      dInput;                                                     /*              -                                   */

    /* internal variables   */
    etFilterType                etFilter;
    double                      dN[3];
    double                      dD[3];
    double                      dOmega;
    double                      dQualityFactor;
    double                      dCa;
    double                      dCb;
    double                      dCc;
    double                      dCd;
    double                      dCe;
    double                      dPrevInput1;
    double                      dPrevInput2;
    double                      dPrevOutput1;
    double                      dPrevOutput2;

    /* output variables     */
    double                      dOutput;

    bool                        Init(void);

    bool                        Load(void);
};

#endif /* _TRANSFERFNC_H_ */
