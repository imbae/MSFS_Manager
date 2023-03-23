/*
 *  transferFnc.cpp
 *
 *  Created on: 2023. 03. 19.
 *  Author: JSH
 */

#include "transferFnc.h"

/**************************************************************************************************/

transferFnc::transferFnc(void)
{
    Init();
    Load();
}

/**************************************************************************************************/

transferFnc::~transferFnc(void)
{
}

/**************************************************************************************************/

bool transferFnc::Init(void)
{
    /* load variables       */


    /* internal variables   */
    for (int i = 0; i < 3; i++)
    {
        dN[i];
        dD[i];
    }

    dOmega = 0.0;
    dQualityFactor = 0.0;
    dCa = 0.0;
    dCb = 0.0;
    dCc = 0.0;
    dCd = 0.0;
    dCe = 0.0;
    dPrevInput1 = 0.0;
    dPrevInput2 = 0.0;
    dPrevOutput1 = 0.0;
    dPrevOutput2 = 0.0;

    /* input variables      */
    dDampingRatio = 0.0;
    dCutOffFrequency = 0.0;
    dInput = 0.0;

    /* output variables     */
    dOutput = 0.0;

    return true;
}

/**************************************************************************************************/

bool transferFnc::Load(void)
{
    return true;
}

/**************************************************************************************************/

bool transferFnc::Update(void)
{
    dOutput = dInput * dCa + dPrevInput1 * dCb + dPrevInput2 * dCc
        - dPrevOutput1 * dCd - dPrevOutput2 * dCe;

    dPrevInput2 = dPrevInput1;
    dPrevInput1 = dInput;
    dPrevOutput2 = dPrevOutput1;
    dPrevOutput1 = dOutput;

    return true;
}

/**************************************************************************************************/

void transferFnc::MakeFilterTransferFunction(etFilterType type)
{
    /*
    *  Make S-Domain TransferFunction as follow :
    * 
    *         n2*S^(2) + n1*S^(1) + n0
    *  H(S) = -------------------------
    *         d2*S^(2) + d1*S^(1) + d0
    * 
    */

    dOmega = 2.0 * PI * dCutOffFrequency;
    dQualityFactor = 1.0 / (2.0 * dDampingRatio);

    switch (type)
    {
        case eLPF_1stOrder:
            dN[2] = 0.0;
            dN[1] = 0.0;
            dN[0] = dOmega;
            dD[2] = 0.0;
            dD[1] = 1.0;
            dD[0] = dOmega;
            break;
        case eLPF_2ndOrder:
            dN[2] = 0.0;
            dN[1] = 0.0;
            dN[0] = dOmega * dOmega;
            dD[2] = 1.0;
            dD[1] = dOmega / dQualityFactor;
            dD[0] = dOmega * dOmega;
            break;
        case eHPF_1stOrder:
            dN[2] = 0.0;
            dN[1] = 1.0;
            dN[0] = 0.0;
            dD[2] = 0.0;
            dD[1] = 1.0;
            dD[0] = dOmega;
            break;
        case eHPF_2ndOrder:
            dN[2] = 1.0;
            dN[1] = 0.0;
            dN[0] = 0.0;
            dD[2] = 1.0;
            dD[1] = dOmega / dQualityFactor;
            dD[0] = dOmega * dOmega;
            break;
        case eIntegrator:
            dN[2] = 0.0;
            dN[1] = 0.0;
            dN[0] = 1.0;
            dD[2] = 0.0;
            dD[1] = 1.0;
            dD[0] = 0.0;
            break;
        default:
            cerr << "Unknown filter type" << endl;
            break;
    }

    /*
    *  Make Z-Domain TransferFunction as follow :
    * 
    *           Ca + Cb*Z^(-1) + Cc*Z^(-2)
    *  H(S) = -----------------------------
    *          1.0 + Cd*Z^(-1) + Ce*Z^(-2)
    * 
    */

    double _dDenom = 4.0 * dD[2] + 2.0 * dD[1] * DELTA_T + dD[0] * DELTA_T * DELTA_T;

    dCa = ( 4.0 * dN[2] + 2.0 * dN[1] * DELTA_T + dN[0] * DELTA_T * DELTA_T) / _dDenom;
    dCb = (-8.0 * dN[2] + 2.0 * dN[0] * DELTA_T * DELTA_T) / _dDenom;
    dCc = ( 4.0 * dN[2] - 2.0 * dN[1] * DELTA_T + dN[0] * DELTA_T * DELTA_T) / _dDenom;
    dCd = (-8.0 * dD[2] + 2.0 * dD[0] * DELTA_T * DELTA_T) / _dDenom;
    dCe = ( 4.0 * dD[2] - 2.0 * dD[1] * DELTA_T + dD[0] * DELTA_T * DELTA_T) / _dDenom;

}

