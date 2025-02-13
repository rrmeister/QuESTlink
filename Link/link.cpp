/* @file
 * Serves as the main link or interface between C++ and QuEST to Mathematica.
 * Note functions defined in other files may directly communicate with the 
 * Mathematica process too. 
 * 
 * The functions defined in these files are grouped as either 'local', 
 * 'callable', 'wrapper', 'internal':
 * - 'local' functions are called from only inside this file.
 * - 'callable' functions are directly callable by a user in MMA.
 * - 'wrapper' functions wrap a specific QuEST function and are directly 
 *    callable by a user in MMA.
 * - 'internal' functions are callable by the MMA, but shouldn't be called
 *    directly by a user - instead, they are wrapped by MMA-defined functions
 * 
 * Note the actual function names called from the MMA code differ from those 
 * below, as specified in quest_templates.tm
 *
 * @author Tyson Jones
 */

#include "wstp.h"
#include "QuEST.h"

#include "link.hpp"
#include "errors.hpp"
#include "decoders.hpp"
#include "extensions.hpp"
#include "circuits.hpp"
#include "derivatives.hpp"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <exception>





QuESTEnv env;
std::vector<Qureg> quregs;
std::vector<bool> quregIsCreated;




/* 
 * QUREG MANAGEMENT
 */

size_t local_getNextQuregID(void) {
    size_t id;
    
    // check for next id
    for (id=0; id < quregs.size(); id++)
        if (!quregIsCreated[id])
            return id;
            
    // if none are available, make more space (using a blank Qureg)
    Qureg blank;
    id = quregs.size();
    quregs.push_back(blank);
    quregIsCreated.push_back(false);
    return id;
}

void wrapper_createQureg(int numQubits) {
    try { 
        size_t id = local_getNextQuregID();
        quregs[id] = createQureg(numQubits, env); // throws
        quregIsCreated[id] = true;
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CreateQureg", err.message);
    }
}

void wrapper_createDensityQureg(int numQubits) {
    try { 
        size_t id = local_getNextQuregID();
        quregs[id] = createDensityQureg(numQubits, env); // throws
        quregIsCreated[id] = true;
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CreateDensityQureg", err.message);
    }
}

void wrapper_destroyQureg(int id) {
    try { 
        local_throwExcepIfQuregNotCreated(id); // throws
        
        destroyQureg(quregs[id], env); // throws
        quregIsCreated[id] = false;
        WSPutInteger(stdlink, id);

    } catch( QuESTException& err) {
        local_sendErrorAndFail("DestroyQureg", err.message);
    }
}

void callable_destroyAllQuregs(void) {
    
    for (size_t id=0; id < quregs.size(); id++) {
        if (quregIsCreated[id]) {
            destroyQureg(quregs[id], env);
            quregIsCreated[id] = false;
        }
    }
    WSPutSymbol(stdlink, "Null");
}

void callable_createQuregs(int numQubits, int numQuregs) {
    int* ids = NULL;
  
    try { 
        if (numQuregs < 0)
            throw QuESTException("", "Invalid number of quregs. Must be >= 0."); // throws
        
        ids = (int*) malloc(numQuregs * sizeof *ids);
        
        for (int i=0; i < numQuregs; i++) {
            int id = local_getNextQuregID();
            ids[i] = id;
            quregs[id] = createQureg(numQubits, env); // throws (first; no cleanup needed)
            quregIsCreated[id] = true;
        }
        WSPutIntegerList(stdlink, ids, numQuregs);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CreateQuregs", err.message);
    }
    
    // clean-up (control flow WILL return here after local_sendErrorAndFail)
    if (ids != NULL)
        free(ids);
}

void callable_createDensityQuregs(int numQubits, int numQuregs) {
    int* ids = NULL;
  
    try {
        if (numQuregs < 0)
            throw QuESTException("", "Invalid number of quregs. Must be >= 0."); // throws
        
        ids = (int*) malloc(numQuregs * sizeof *ids);
            
        for (int i=0; i < numQuregs; i++) {
            int id = local_getNextQuregID();
            ids[i] = id;
            quregs[id] = createDensityQureg(numQubits, env); // throws (first; no cleanup needed)
            quregIsCreated[id] = true;
        }
        WSPutIntegerList(stdlink, ids, numQuregs);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CreateDensityQuregs", err.message);
    }
    
    // clean-up (control flow WILL return here after local_sendErrorAndFail)
    if (ids != NULL)
        free(ids);
}




/*
 * QASM
 */

void callable_startRecordingQASM(int id) {
    try { 
        local_throwExcepIfQuregNotCreated(id); // throws
        Qureg qureg = quregs[id];
        startRecordingQASM(qureg);
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("StartRecordingQASM", err.message);
    }
}

void callable_stopRecordingQASM(int id) {
    try { 
        local_throwExcepIfQuregNotCreated(id); // throws
        Qureg qureg = quregs[id];
        stopRecordingQASM(qureg);
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("StopRecordingQASM", err.message);
    }
}

void callable_clearRecordedQASM(int id) {
    try { 
        local_throwExcepIfQuregNotCreated(id); // throws
        Qureg qureg = quregs[id];
        clearRecordedQASM(qureg);
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("ClearRecordedQASM", err.message);
    }
}

void callable_getRecordedQASM(int id) {
    try { 
        local_throwExcepIfQuregNotCreated(id); // throws
        Qureg qureg = quregs[id];
        char* buf =  qureg.qasmLog->buffer;
        WSPutString(stdlink, buf);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("GetRecordedQASM", err.message);
    }
}




/*
 * GETTERS
 */

void internal_getAmp(int quregID) {
    
    // get args from MMA (must do this before possible early-exit)
    wsint64 rawRow, rawCol;
    WSGetInteger64(stdlink, &rawRow);
    WSGetInteger64(stdlink, &rawCol);
    
    // explicitly cast from wsint64 type, which resolves to a different C++ primitive 
    // depending compiler (MSVC: long long int, GNU: long int)
    long long int row = (long long int) rawRow;
    long long int col = (long long int) rawCol;
    
    try { 
        local_throwExcepIfQuregNotCreated(quregID); // throws
        
        // ensure user supplied the correct number of args for the qureg type
        Qureg qureg = quregs[quregID];
        if (qureg.isDensityMatrix && col==-1)
            throw QuESTException("", "Called on a density matrix without supplying both row and column."); // throws
        if (!qureg.isDensityMatrix && col!=-1)
            throw QuESTException("", "Called on a state-vector, yet a meaningless column index was supplied."); // throws
        
        // fetch amp (can throw internal QuEST errors)
        Complex amp;
        if (qureg.isDensityMatrix)
            amp = getDensityAmp(qureg, row, col); // throws
        else
            amp = getAmp(qureg, row); // throws
        
        // return as complex number
        WSPutFunction(stdlink, "Complex", 2);
        WSPutReal64(stdlink, amp.real);
        WSPutReal64(stdlink, amp.imag);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("GetAmp", err.message);
    }
}

void callable_isDensityMatrix(int quregID) {
    try { 
        local_throwExcepIfQuregNotCreated(quregID); // throws
        Qureg qureg = quregs[quregID];
        WSPutInteger(stdlink, qureg.isDensityMatrix);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("IsDensityMatrix", err.message);
    }
}

/* puts a Qureg into MMA, with the structure of
 * {numQubits, isDensityMatrix, realAmps, imagAmps}.
 * Instead gives -1 if error (e.g. qureg id is wrong)
 */
void internal_getQuregMatrix(int id) {
    
    // superflous try-catch, but we want to grab the quregNotCreated error message
    // (and this also keeps the pattern consistent)
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
    
        Qureg qureg = quregs[id];
        syncQuESTEnv(env);       // does nothing on local
        copyStateFromGPU(qureg); // does nothing on CPU
        
        WSPutFunction(stdlink, "List", 4);
        WSPutInteger(stdlink, qureg.numQubitsRepresented);
        WSPutInteger(stdlink, qureg.isDensityMatrix);
        WSPutReal64List(stdlink, qureg.stateVec.real, qureg.numAmpsTotal);
        WSPutReal64List(stdlink, qureg.stateVec.imag, qureg.numAmpsTotal);    
        
    } catch (QuESTException& err) {
        local_sendErrorAndFail("GetQuregMatrix", err.message);
    }
}

/* Returns a list of all created quregs
 */
void callable_getAllQuregs(void) {
    
    // collect all created quregs
    int numQuregs = 0;
    int* idList = (int*) malloc(quregs.size() * sizeof *idList);
    for (size_t id=0; id < quregs.size(); id++)
        if (quregIsCreated[id])
            idList[numQuregs++] = id;
    
    WSPutIntegerList(stdlink, idList, numQuregs);
    free(idList);
}




/*
 * STATE INITIALISATION
 */

void wrapper_initZeroState(int id) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        initZeroState(quregs[id]);
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("InitZeroState", err.message);
    }
}

void wrapper_initPlusState(int id) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        initPlusState(quregs[id]);
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("InitPlusState", err.message);
    }
}

void wrapper_initClassicalState(int id, int stateInd) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        initClassicalState(quregs[id], stateInd); // throws
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("InitClassicalState", err.message);
    }
}

void wrapper_initPureState(int quregID, int pureID) {
    try {
        local_throwExcepIfQuregNotCreated(quregID); // throws
        local_throwExcepIfQuregNotCreated(pureID); // throws
        initPureState(quregs[quregID], quregs[pureID]); // throws
        WSPutInteger(stdlink, quregID);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("InitPureState", err.message);
    }
}

void wrapper_initStateFromAmps(int quregID, qreal* reals, long l1, qreal* imags, long l2) {
    try {
        local_throwExcepIfQuregNotCreated(quregID); // throws
        
        Qureg qureg = quregs[quregID];
        if (l1 != l2 || l1 != qureg.numAmpsTotal)
            throw QuESTException("", "incorrect number of amplitudes supplied. State has not been changed."); // throws
        
        initStateFromAmps(qureg, reals, imags); // possibly throws?
        WSPutInteger(stdlink, quregID);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("InitStateFromAmps", err.message);
    }
}

void wrapper_cloneQureg(int outID, int inID) {
    try {
        local_throwExcepIfQuregNotCreated(outID); // throws
        local_throwExcepIfQuregNotCreated(inID); // throws
        cloneQureg(quregs[outID], quregs[inID]); // throws
        WSPutInteger(stdlink, outID);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CloneQureg", err.message);
    }
}




/*
 * STATE MODIFICATION
 */

void wrapper_collapseToOutcome(int id, int qb, int outcome) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        collapseToOutcome(quregs[id], qb, outcome); // throws 
        WSPutInteger(stdlink, id);
    
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CollapseToOutcome", err.message);
    }
}

void internal_setWeightedQureg(
    double facRe1, double facIm1, int qureg1, 
    double facRe2, double facIm2, int qureg2, 
    double facReOut, double facImOut, int outID
) {
    try {
        local_throwExcepIfQuregNotCreated(qureg1); // throws
        local_throwExcepIfQuregNotCreated(qureg2); // throws
        local_throwExcepIfQuregNotCreated(outID); // throws
        
        // verbose for MSVC  :(
        Complex fac1, fac2, facOut;
        fac1.real=facRe1; fac1.imag=facIm1;
        fac2.real=facRe2; fac2.imag=facIm2;
        facOut.real=facReOut; facOut.imag=facImOut;
        setWeightedQureg(
            fac1, quregs[qureg1],
            fac2, quregs[qureg2],
            facOut, quregs[outID]); // throws
        
        WSPutInteger(stdlink, outID);
        
    } catch (QuESTException& err) {
        local_sendErrorAndFail("SetWeightedQureg", err.message);
    }
}

void internal_setAmp(int quregID, double ampRe, double ampIm) {
    
    // get args from MMA (must do this before possible early-exit)
    wsint64 rawRow, rawCol;
    WSGetInteger64(stdlink, &rawRow);
    WSGetInteger64(stdlink, &rawCol); // -1 for state-vecs
    
    // explicitly cast from wsint64 type, which resolves to a different C++ primitive 
    // depending compiler (MSVC: long long int, GNU: long int)
    long long int row = (long long int) rawRow;
    long long int col = (long long int) rawCol;
    
    try { 
        local_throwExcepIfQuregNotCreated(quregID); // throws
        
        // ensure user supplied the correct number of args for the qureg type
        Qureg qureg = quregs[quregID];
        if (qureg.isDensityMatrix && col==-1)
            throw QuESTException("", "Called on a density matrix without supplying both row and column."); // throws
        if (!qureg.isDensityMatrix && col!=-1)
            throw QuESTException("", "Called on a state-vector, yet a meaningless column index was supplied."); // throws
        
        // fetch amp (can throw internal QuEST errors)
        if (qureg.isDensityMatrix)
            setDensityAmps(qureg, row, col, &ampRe, &ampIm, 1);
        else
            setAmps(qureg, row, &ampRe, &ampIm, 1);

        WSPutInteger(stdlink, quregID);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("SetAmp", err.message);
    }
}




/*
 * DECOHERENCE
 */

void wrapper_mixDephasing(int id, int qb1, qreal prob) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        mixDephasing(quregs[id], qb1, prob); // throws
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("MixDephasing", err.message);
    }
}

void wrapper_mixTwoQubitDephasing(int id, int qb1, int qb2, qreal prob) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        mixTwoQubitDephasing(quregs[id], qb1, qb2, prob); // throws
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("MixTwoQubitDephasing", err.message);
    }
}

void wrapper_mixDepolarising(int id, int qb1, qreal prob) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        mixDepolarising(quregs[id], qb1, prob); // throws
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("MixDepolarising", err.message);
    }
}

void wrapper_mixTwoQubitDepolarising(int id, int qb1, int qb2, qreal prob) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        mixTwoQubitDepolarising(quregs[id], qb1, qb2, prob); // throws
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("MixTwoQubitDepolarising", err.message);
    }
}

void wrapper_mixDamping(int id, int qb, qreal prob) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        mixDamping(quregs[id], qb, prob); // throws
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("MixDamping", err.message);
    }
}




/*
 * CALCULATIONS
 */

void wrapper_calcProbOfOutcome(int id, int qb, int outcome) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        qreal prob = calcProbOfOutcome(quregs[id], qb, outcome); // throws
        WSPutReal64(stdlink, prob); 
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CalcProbOfOutCome", err.message);
    }
}

void wrapper_calcProbOfAllOutcomes(int id, int* qubits, long numQubits) {
    
    long long int numProbs = (1LL << numQubits);
    qreal* probs = (numQubits > 0)? (qreal*) malloc(numProbs * sizeof* probs) : NULL;
    
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        calcProbOfAllOutcomes(probs, quregs[id], qubits, numQubits); // throws
        
        WSPutReal64List(stdlink, probs, numProbs);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CalcProbOfAllOutcomes", err.message);
    }
    
    if (numQubits > 0)
        free(probs);
}

void wrapper_calcFidelity(int id1, int id2) {
    try {
        local_throwExcepIfQuregNotCreated(id1); // throws
        local_throwExcepIfQuregNotCreated(id2); // throws
        qreal fid = calcFidelity(quregs[id1], quregs[id2]); // throws
        WSPutReal64(stdlink, fid);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CalcFidelity", err.message);
    }
}

void wrapper_calcInnerProduct(int id1, int id2) {
    try {
        local_throwExcepIfQuregNotCreated(id1); // throws
        local_throwExcepIfQuregNotCreated(id2); // throws
        Complex res = calcInnerProduct(quregs[id1], quregs[id2]); // throws
        WSPutFunction(stdlink, "Complex", 2);
        WSPutReal64(stdlink, res.real);
        WSPutReal64(stdlink, res.imag);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CalcInnerProduct", err.message);
    }
}

void wrapper_calcDensityInnerProduct(int id1, int id2) {
    try {
        local_throwExcepIfQuregNotCreated(id1); // throws
        local_throwExcepIfQuregNotCreated(id2); // throws
        qreal res = calcDensityInnerProduct(quregs[id1], quregs[id2]); // throws
        WSPutReal64(stdlink, res);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CalcDensityInnerProduct", err.message);
    }
}

void wrapper_calcPurity(int id) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        qreal pur = calcPurity(quregs[id]); // throws
        WSPutReal64(stdlink, pur);
    
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CalcPurity", err.message);
    }
}

void wrapper_calcTotalProb(int id) {
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        qreal prob = calcTotalProb(quregs[id]); // throws
        WSPutReal64(stdlink, prob);
    
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CalcTotalProb", err.message);
    }
}

void wrapper_calcHilbertSchmidtDistance(int id1, int id2) {
    try {
        local_throwExcepIfQuregNotCreated(id1); // throws
        local_throwExcepIfQuregNotCreated(id2); // throws
        qreal dist = calcHilbertSchmidtDistance(quregs[id1], quregs[id2]); // throws
        WSPutReal64(stdlink, dist);
    
    } catch( QuESTException& err) {
        local_sendErrorAndFail("CalcHilbertSchmidtDistance", err.message);
    }
}

/* returns a real vector with ith element 
 * calcDensityInnerProduct(qureg[rhoId], qureg[omegaIds[i]]) 
 */
void internal_calcDensityInnerProductsVector(int rhoId, int* omegaIds, long numOmegas) {
    
    // init to null so we can check whether it needs cleanup
    qreal* prods = NULL;
    
    try {
        local_throwExcepIfQuregNotCreated(rhoId); // throws
        for (int i=0; i<numOmegas; i++)
            local_throwExcepIfQuregNotCreated(omegaIds[i]); // throws
        
        // calculate inner products (must free this)
        prods = (qreal*) malloc(numOmegas * sizeof *prods);
        for (int i=0; i<numOmegas; i++)
            prods[i] = calcDensityInnerProduct(quregs[rhoId], quregs[omegaIds[i]]); // throws
            
        // send result to MMA
        WSPutReal64List(stdlink, prods, numOmegas);
        
        // and clean-up
        free(prods);
    
    } catch (QuESTException& err) {
        
        // may still need to clean-up
        if (prods != NULL)
            free(prods);
        
        local_sendErrorAndFail("CalcDensityInnerProducts", err.message);
    }
}

/* returns vector with ith element <qureg[braId]|qureg[ketIds[i]]> 
 */
void internal_calcInnerProductsVector(int braId, int* ketIds, long numKets) {
    
    // init to NULL so we can check if they need clean-up later
    qreal* vecRe = NULL;
    qreal* vecIm = NULL;
    
    try {
        local_throwExcepIfQuregNotCreated(braId); // throws
        for (int i=0; i<numKets; i++)
            local_throwExcepIfQuregNotCreated(ketIds[i]); // throws
            
        // calculate inner products (must free these)
        vecRe = (qreal*) malloc(numKets * sizeof *vecRe);
        vecIm = (qreal*) malloc(numKets * sizeof *vecIm);
        
        for (int i=0; i<numKets; i++) {
            Complex val = calcInnerProduct(quregs[braId], quregs[ketIds[i]]); // throws
            vecRe[i] = val.real;
            vecIm[i] = val.imag;
        }
        
        // send result to MMA
        WSPutFunction(stdlink, "List", 2);
        WSPutReal64List(stdlink, vecRe, numKets);
        WSPutReal64List(stdlink, vecIm, numKets);
        
        // clean-up
        free(vecRe);
        free(vecIm);
        
    } catch (QuESTException& err) {
            
        // may still need to clean-up
        if (vecRe != NULL)
            free(vecRe);
        if (vecIm != NULL)
            free(vecIm);
        
        local_sendErrorAndFail("CalcInnerProducts", err.message);
    }
}

/* returns real, symmetric matrix with ith jth element calcDensityInnerProduct(quregs[quregIds[i]], qureg[qurgIds[j]]) */
void internal_calcDensityInnerProductsMatrix(int* quregIds, long numQuregs) {
    
    // init to NULL so we can later check if it needs cleanup
    qreal* matr = NULL;
    
    try {
        // check all quregs are created
        for (int i=0; i<numQuregs; i++)
            local_throwExcepIfQuregNotCreated(quregIds[i]); // throws
            
        // store real matrix as `nested pointers`
        long len = numQuregs * numQuregs;
        matr = (qreal*) malloc(len * sizeof *matr);
        
        // compute matrix (exploiting matrix is symmetric)
        for (int r=0; r<numQuregs; r++) {
            for (int c=0; c<numQuregs; c++) {
                if (c >= r) {
                    qreal val = calcDensityInnerProduct(quregs[quregIds[r]], quregs[quregIds[c]]); // throws
                    matr[r*numQuregs + c] = val;
                } else {
                    matr[r*numQuregs + c] = matr[c*numQuregs + r];
                }
            }
        }
        
        // return
        WSPutReal64List(stdlink, matr, len);
        
        // clean-up
        free(matr);
        
    } catch (QuESTException& err) {
        
        // may still need clean-up
        if (matr != NULL)
            free(matr);
            
        // send error and exit
        local_sendErrorAndFail("CalcDensityInnerProducts", err.message);
    }
}

/* returns Hermitian matrix with ith jth element <qureg[i]|qureg[j]> */
void internal_calcInnerProductsMatrix(int* quregIds, long numQuregs) {
    
    // init to NULL so we can later check if it needs cleanup
    qreal* matrRe = NULL;
    qreal* matrIm = NULL;
    
    try {
        // check all quregs are created
        for (int i=0; i<numQuregs; i++)
            local_throwExcepIfQuregNotCreated(quregIds[i]); // throws
    
        // store complex matrix as 2 flat real arrays
        long len = numQuregs * numQuregs;
        matrRe = (qreal*) malloc(len * sizeof *matrRe);
        matrIm = (qreal*) malloc(len * sizeof *matrIm);
    
        // compute elems, exploiting matrix is Hermitian
        for (int r=0; r<numQuregs; r++) {
            for (int c=0; c<numQuregs; c++) {
                if (c >= r) {
                    Complex val = calcInnerProduct(quregs[quregIds[r]], quregs[quregIds[c]]); // throws
                    matrRe[r*numQuregs + c] = val.real;
                    matrIm[r*numQuregs + c] = val.imag;
                } else {
                    matrRe[r*numQuregs + c] =   matrRe[c*numQuregs + r];
                    matrIm[r*numQuregs + c] = - matrIm[c*numQuregs + r]; // conjugate transpose
                }
            }
        }
        
        // return
        WSPutFunction(stdlink, "List", 2);
        WSPutReal64List(stdlink, matrRe, len);
        WSPutReal64List(stdlink, matrIm, len);
        
        // cleanup
        free(matrRe);
        free(matrIm);
        
    } catch (QuESTException& err) {
        
        // may still need to cleanup
        if (matrRe != NULL)
            free(matrRe);
        if (matrIm != NULL)
            free(matrIm);
            
        local_sendErrorAndFail("CalcInnerProducts", err.message);
    }
}




/*
 * CIRCUIT EXECUTION 
 */












/*
 * HAMILTONIAN EVALUATION
 */



/* Evaluates the expected value of a Pauli product */
void internal_calcExpecPauliProd(int quregId, int workspaceId) {
    
    // get arguments from MMA link before validation (must be freed)
    int numPaulis;
    int *pauliIntCodes;
    WSGetInteger32List(stdlink, &pauliIntCodes, &numPaulis);
    int *targs;
    WSGetInteger32List(stdlink, &targs, &numPaulis);
    
    enum pauliOpType* pauliCodes = NULL;
    
    try {
        local_throwExcepIfQuregNotCreated(quregId); // throws 
        local_throwExcepIfQuregNotCreated(workspaceId); // throws
        
        if (quregId == workspaceId)
            throw QuESTException("", "qureg and workspace must be different quregs.");
        
        Qureg qureg = quregs[quregId];
        Qureg workspace = quregs[workspaceId];
        
        // recast pauli codes (must free)
        pauliCodes = (enum pauliOpType*) malloc(numPaulis * sizeof *pauliCodes);
        for (int i=0; i<numPaulis; i++)
            pauliCodes[i] = (pauliOpType) pauliIntCodes[i];
            
        qreal prod = calcExpecPauliProd(qureg, targs, pauliCodes, numPaulis, workspace); // throws
        WSPutReal64(stdlink, prod);
        
        // clean-up
        WSReleaseInteger32List(stdlink, pauliIntCodes, numPaulis);
        WSReleaseInteger32List(stdlink, targs, numPaulis);
        if (pauliCodes != NULL)
            free(pauliCodes);
        
    } catch (QuESTException& err) {
        
        // must still clean-up
        WSReleaseInteger32List(stdlink, pauliIntCodes, numPaulis);
        WSReleaseInteger32List(stdlink, targs, numPaulis);
        if (pauliCodes != NULL)
            free(pauliCodes);
        
        local_sendErrorAndFail("CalcExpecPauliProd", err.message);
    }
}

void internal_calcExpecPauliSum(int quregId, int workspaceId) {
    
    // must load MMA args before validation (these must all also be freed)
    int numPaulis, numTerms;
    qreal* termCoeffs;
    int *allPauliCodes, *allPauliTargets, *numPaulisPerTerm;
    local_loadEncodedPauliSumFromMMA(
        &numPaulis, &numTerms, &termCoeffs, &allPauliCodes, &allPauliTargets, &numPaulisPerTerm);
        
    // init to null in case loading fails, to indicate no-cleanup needed
    pauliOpType* arrPaulis = NULL;
        
    try {
        // ensure quregs exist
        local_throwExcepIfQuregNotCreated(quregId); // throws
        local_throwExcepIfQuregNotCreated(workspaceId); // throws
        
        if (quregId == workspaceId)
            throw QuESTException("", "qureg and workspace must be different quregs.");
        
        Qureg qureg = quregs[quregId];
        Qureg workspace = quregs[workspaceId];
        
        // reformat MMA args into QuEST Hamil format (must be later freed)
        arrPaulis = local_decodePauliSum(
            qureg.numQubitsRepresented, numTerms, allPauliCodes, allPauliTargets, numPaulisPerTerm); // throws
        
        // compute return value
        qreal val = calcExpecPauliSum(qureg, arrPaulis, termCoeffs, numTerms, workspace); // throws
        WSPutReal64(stdlink, val);
        
        // cleanup
        local_freePauliSum(numPaulis, numTerms, 
            termCoeffs, allPauliCodes, allPauliTargets, numPaulisPerTerm, arrPaulis);
    
    } catch( QuESTException& err) {
        
        // must still perform cleanup to avoid memory leak
        // (arrPaulis may still be NULL, even though other data is populated)
        local_freePauliSum(numPaulis, numTerms, 
            termCoeffs, allPauliCodes, allPauliTargets, numPaulisPerTerm, arrPaulis);

        local_sendErrorAndFail("CalcExpecPauliSum", err.message);
        return;
    }
}

void internal_calcPauliSumMatrix(int numQubits) {
    
    // must load MMA args before validation (these must all also be freed)
    int numPaulis, numTerms;
    qreal* termCoeffs;
    int *allPauliCodes, *allPauliTargets, *numPaulisPerTerm;
    local_loadEncodedPauliSumFromMMA(
        &numPaulis, &numTerms, &termCoeffs, &allPauliCodes, &allPauliTargets, &numPaulisPerTerm);

    // create states needed to apply Pauli products
    Qureg inQureg = createQureg(numQubits, env);
    Qureg outQureg = createQureg(numQubits, env);
    
    // init to null in case loading fails, to indicate no-cleanup needed
    pauliOpType* arrPaulis = NULL;
    
    try {
        // reformat MMA args into QuEST Hamil format (must be later freed)    
        arrPaulis = local_decodePauliSum(
            numQubits, numTerms, allPauliCodes, allPauliTargets, numPaulisPerTerm); // throws
        
        // check that applyPauliSum will succeed (small price to pay; can't interrupt loop!)
        applyPauliSum(inQureg, arrPaulis, termCoeffs, numTerms, outQureg);
        
    } catch( QuESTException& err) {

        // must still perform cleanup to avoid memory leak
        local_freePauliSum(numPaulis, numTerms, 
            termCoeffs, allPauliCodes, allPauliTargets, numPaulisPerTerm, arrPaulis);
        destroyQureg(inQureg, env);
        destroyQureg(outQureg, env);
        
        // then exit 
        local_sendErrorAndFail("CalcPauliSumMatrix", err.message);
        return;
    }
    
    // get result of paulis on each basis state
    long long int dim = inQureg.numAmpsTotal;
    WSPutFunction(stdlink, "List", 2*dim);
    
    for (long long int i=0; i < dim; i++) {
        initClassicalState(inQureg, i);
        applyPauliSum(inQureg, arrPaulis, termCoeffs, numTerms, outQureg);
        
        syncQuESTEnv(env);
        copyStateFromGPU(outQureg); // does nothing on CPU
        
        WSPutReal64List(stdlink, outQureg.stateVec.real, dim);
        WSPutReal64List(stdlink, outQureg.stateVec.imag, dim);
    }
    
    // output has already been 'put'

    // clean up
    local_freePauliSum(numPaulis, numTerms, 
        termCoeffs, allPauliCodes, allPauliTargets, numPaulisPerTerm, arrPaulis);
    destroyQureg(inQureg, env);
    destroyQureg(outQureg, env);
}

void internal_applyPauliSum(int inId, int outId) {
    
    // must load MMA args before validation (these must all also be freed)
    int numPaulis, numTerms;
    qreal* termCoeffs;
    int *allPauliCodes, *allPauliTargets, *numPaulisPerTerm;
    local_loadEncodedPauliSumFromMMA(
        &numPaulis, &numTerms, &termCoeffs, &allPauliCodes, &allPauliTargets, &numPaulisPerTerm);

    // init to null in case loading fails, to indicate no-cleanup needed
    pauliOpType* arrPaulis = NULL;
    
    try {
        local_throwExcepIfQuregNotCreated(inId); // throws
        local_throwExcepIfQuregNotCreated(outId); // throws
        
        if (inId == outId)
            throw QuESTException("", "inQureg and outQureg must be different quregs.");
        
        Qureg inQureg = quregs[inId];
        Qureg outQureg = quregs[outId];
        
        // reformat MMA args into QuEST Hamil format (must be later freed)
        arrPaulis = local_decodePauliSum(
            inQureg.numQubitsRepresented, numTerms, allPauliCodes, allPauliTargets, numPaulisPerTerm); // throws
        
        applyPauliSum(inQureg, arrPaulis, termCoeffs, numTerms, outQureg); // throws
        
        // cleanup
        local_freePauliSum(numPaulis, numTerms, 
            termCoeffs, allPauliCodes, allPauliTargets, numPaulisPerTerm, arrPaulis);
            
        // and return
        WSPutInteger(stdlink, outId);
        
    } catch( QuESTException& err) {
        
        // must still clean-up (arrPaulis may still be NULL)
        local_freePauliSum(numPaulis, numTerms, 
            termCoeffs, allPauliCodes, allPauliTargets, numPaulisPerTerm, arrPaulis);
            
        // and report error
        local_sendErrorAndFail("ApplyPauliSum", err.message);
    }
}

void internal_applyPhaseFunc(int quregId, int* qubits, long numQubits, int encoding) {
    
    // fetch args into dynamic memory
    qreal* coeffs;
    qreal* exponents;
    int numTerms;
    long long int* overrideInds;
    wsint64* ws_overrideInds;
    qreal* overridePhases;
    int numOverrides;
    WSGetReal64List(stdlink, &coeffs, &numTerms);
    WSGetReal64List(stdlink, &exponents, &numTerms);
    WSGetInteger64List(stdlink, &ws_overrideInds, &numOverrides);
    WSGetReal64List(stdlink, &overridePhases, &numOverrides);
    
    // cast Wolfram 'wsint' into QuEST 'long long int'
    overrideInds = (long long int*) malloc(numOverrides * sizeof(long long int));
    for (int i=0; i<numOverrides; i++)
        overrideInds[i] = (long long int) ws_overrideInds[i];
    
    try {
        local_throwExcepIfQuregNotCreated(quregId); // throws
        Qureg qureg = quregs[quregId];
        
        applyPhaseFuncOverrides(qureg, qubits, numQubits, (enum bitEncoding) encoding, coeffs, exponents, numTerms, overrideInds, overridePhases, numOverrides); // throws
        
        WSPutInteger(stdlink, quregId);
        
    } catch (QuESTException& err) {
        local_sendErrorAndFail("ApplyPhaseFunc", err.message);
    }
    
    // clean-up (even if error)
    WSReleaseReal64List(stdlink, coeffs, numTerms);
    WSReleaseReal64List(stdlink, exponents, numTerms);
    WSReleaseInteger64List(stdlink, ws_overrideInds, numOverrides);
    WSReleaseReal64List(stdlink, overridePhases, numOverrides);
    free(overrideInds);
}

void internal_applyMultiVarPhaseFunc(int quregId) {
    // 86% of this function is restructuring arguments... despicable
    
    // fetch flat-packed args
    int* qubits;
    int* numQubitsPerReg;
    int numRegs;
    int encoding;
    qreal* coeffs;
    qreal* exponents;
    int* numTermsPerReg;
    wsint64* ws_overrideInds;
    qreal* overridePhases;
    int numOverrides;
    // (irrelevant flattened list lengths)
    int dummy_totalQubits;
    int dummy_totalTerms;
    int dummy_totalInds;
    WSGetInteger32List(stdlink, &qubits, &dummy_totalQubits);
    WSGetInteger32List(stdlink, &numQubitsPerReg, &numRegs);
    WSGetInteger32(stdlink, &encoding);
    WSGetReal64List(stdlink, &coeffs, &dummy_totalTerms);
    WSGetReal64List(stdlink, &exponents, &dummy_totalTerms);
    WSGetInteger32List(stdlink, &numTermsPerReg, &numRegs);
    WSGetInteger64List(stdlink, &ws_overrideInds, &dummy_totalInds);
    WSGetReal64List(stdlink, &overridePhases, &numOverrides);
    
    // convert wsint64 arr to long long int 
    long long int* overrideInds = (long long int*) malloc(numOverrides * numRegs * sizeof *overrideInds);
    for (int i=0; i<numOverrides*numRegs; i++)
        overrideInds[i] = (long long int) ws_overrideInds[i];

    try {
        local_throwExcepIfQuregNotCreated(quregId); // throws
        Qureg qureg = quregs[quregId];
        
        applyMultiVarPhaseFuncOverrides(qureg, qubits, numQubitsPerReg, numRegs, (enum bitEncoding) encoding, coeffs, exponents, numTermsPerReg, overrideInds, overridePhases, numOverrides);
        
        WSPutInteger(stdlink, quregId);
        
    } catch (QuESTException& err) {
        
        // execution will proceed to clean-up even if error
        local_sendErrorAndFail("ApplyPhaseFunc", err.message);
    }
    
    // free args
    free(overrideInds);
    WSReleaseInteger32List(stdlink, qubits, dummy_totalQubits);
    WSReleaseInteger32List(stdlink, numQubitsPerReg, numRegs);
    WSReleaseReal64List(stdlink, coeffs, dummy_totalTerms);
    WSReleaseReal64List(stdlink, exponents, dummy_totalTerms);
    WSReleaseInteger32List(stdlink, numTermsPerReg, numRegs);
    WSReleaseInteger64List(stdlink, ws_overrideInds, dummy_totalInds);
    WSReleaseReal64List(stdlink, overridePhases, numOverrides);
}

void internal_applyNamedPhaseFunc(int quregId) {
    // 86% of this function is restructuring arguments... despicable
    
    // fetch flat-packed args
    int* qubits;
    int* numQubitsPerReg;
    int numRegs;
    int encoding;
    int funcNameCode;
    wsint64* ws_overrideInds;
    qreal* overridePhases;
    int numOverrides;
    int dummy_totalQubits; // (irrelevant flattened list lengths)
    int dummy_totalInds; // (irrelevant flattened list lengths)
    WSGetInteger32List(stdlink, &qubits, &dummy_totalQubits);
    WSGetInteger32List(stdlink, &numQubitsPerReg, &numRegs);
    WSGetInteger32(stdlink, &encoding);
    WSGetInteger32(stdlink, &funcNameCode);
    WSGetInteger64List(stdlink, &ws_overrideInds, &dummy_totalInds);
    WSGetReal64List(stdlink, &overridePhases, &numOverrides);

    // convert wsint64 arr to long long int 
    long long int* overrideInds = (long long int*) malloc(numOverrides * numRegs * sizeof *overrideInds);
    for (int i=0; i<numOverrides*numRegs; i++)
        overrideInds[i] = (long long int) ws_overrideInds[i];
            
    try {
        local_throwExcepIfQuregNotCreated(quregId); // throws
        Qureg qureg = quregs[quregId];
        
        applyNamedPhaseFuncOverrides(qureg, qubits, numQubitsPerReg, numRegs, (enum bitEncoding) encoding, (enum phaseFunc) funcNameCode, overrideInds, overridePhases, numOverrides);
        
        WSPutInteger(stdlink, quregId);
        
    } catch (QuESTException& err) {
        
        // execution will proceed to clean-up even if error
        local_sendErrorAndFail("ApplyPhaseFunc", err.message);
    }
    
    free(overrideInds);
    
    // free flat-packed args
    WSReleaseInteger32List(stdlink, qubits, dummy_totalQubits);
    WSReleaseInteger32List(stdlink, numQubitsPerReg, numRegs);
    WSReleaseInteger64List(stdlink, ws_overrideInds, dummy_totalInds);
    WSReleaseReal64List(stdlink, overridePhases, numOverrides);
}

void internal_applyParamNamedPhaseFunc(int quregId) {
    // 86% of this function is restructuring arguments... despicable
    
    // fetch flat-packed args
    int* qubits;
    int* numQubitsPerReg;
    int numRegs;
    int encoding;
    int funcNameCode;
    qreal* params;
    int numParams;
    wsint64* ws_overrideInds;
    qreal* overridePhases;
    int numOverrides;
    int dummy_totalQubits; // (irrelevant flattened list lengths)
    int dummy_totalInds; // (irrelevant flattened list lengths)
    WSGetInteger32List(stdlink, &qubits, &dummy_totalQubits);
    WSGetInteger32List(stdlink, &numQubitsPerReg, &numRegs);
    WSGetInteger32(stdlink, &encoding);
    WSGetInteger32(stdlink, &funcNameCode);
    WSGetReal64List(stdlink, &params, &numParams);
    WSGetInteger64List(stdlink, &ws_overrideInds, &dummy_totalInds);
    WSGetReal64List(stdlink, &overridePhases, &numOverrides);

    // convert wsint64 arr to long long int 
    long long int* overrideInds = (long long int*) malloc(numOverrides * numRegs * sizeof *overrideInds);
    for (int i=0; i<numOverrides*numRegs; i++)
        overrideInds[i] = (long long int) ws_overrideInds[i];
            
    try {
        local_throwExcepIfQuregNotCreated(quregId); // throws
        Qureg qureg = quregs[quregId];
        
        applyParamNamedPhaseFuncOverrides(qureg, qubits, numQubitsPerReg, numRegs, (enum bitEncoding) encoding, (enum phaseFunc) funcNameCode, params, numParams, overrideInds, overridePhases, numOverrides);
        
        WSPutInteger(stdlink, quregId);
        
    } catch (QuESTException& err) {
        
        // execution will proceed to clean-up even if error
        local_sendErrorAndFail("ApplyPhaseFunc", err.message);
    }
    
    free(overrideInds);
    
    // free flat-packed args
    WSReleaseInteger32List(stdlink, qubits, dummy_totalQubits);
    WSReleaseInteger32List(stdlink, numQubitsPerReg, numRegs);
    WSReleaseReal64List(stdlink, params, numParams);
    WSReleaseInteger64List(stdlink, ws_overrideInds, dummy_totalInds);
    WSReleaseReal64List(stdlink, overridePhases, numOverrides);
}

void wrapper_applyQFT(int id, int* qubits, long numQubits) {
    
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        applyQFT(quregs[id], qubits, numQubits); // throws
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("ApplyQFT", err.message);
    }
}
void wrapper_applyFullQFT(int id) {
    
    try {
        local_throwExcepIfQuregNotCreated(id); // throws
        applyFullQFT(quregs[id]);
        WSPutInteger(stdlink, id);
        
    } catch( QuESTException& err) {
        local_sendErrorAndFail("ApplyQFT", err.message);
    }
}



/*
 * WSTP Launch
 */

#ifndef _WIN32

    int main(int argc, char* argv[]) {
      
        // create the single, global QuEST execution env
        env = createQuESTEnv();
        
        // establish link with MMA
        return WSMain(argc, argv);
    }
    
#else

    int PASCAL WinMain( HINSTANCE hinstCurrent, HINSTANCE hinstPrevious, LPSTR lpszCmdLine, int nCmdShow) {
      
        // create the single, global QuEST execution env
        env = createQuESTEnv();

        // parse Windows args
        char  buff[512];
        char FAR * buff_start = buff;
        char FAR * argv[32];
        char FAR * FAR * argv_end = argv + 32;
        hinstPrevious = hinstPrevious; // suppresses a silly warning
        WSScanString( argv, &argv_end, &lpszCmdLine, &buff_start);
        
        // establish link with MMA
        return WSMain( (int)(argv_end - argv), argv);
    }
    
#endif

