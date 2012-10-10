/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-10-10
 *
 * MATLAB interface to libmseed for writing miniSEED files. All processing
 * should be done in MATLAB before passing 'em to this function. This function
 * should be fairly dump so that errors or time problems are as obvious as
 * possible - as little as possible processing should be done here.
 *
 */

# include <mex.h>
# include <libmseed/libmseed.h>

/* This function is designed to work with one source at the time, it takes
 * a series of batches as input, per batch parameters are:
 *
 *  - Start time
 *  - End time (in case of e.g. fixing a time skew..)
 *  - Number of samples
 *  - Data quality (time and position status)
 *  - Data series
 *
 * This allows for time skews between the batches. The batches should follow
 * each other chronologically.
 *
 * The batches are formatted in one long data series, batches are fixed size,
 * the per batch parameters are passed along in a matrix with a row per batch.
 *
 * Common paramters for all batches:
 *
 *  - Source information (network, station, location, channel)
 *  - Sample rate
 *
 * Other parameters:
 *
 *  - Time tolerance to group batches
 *  - Sample rate tolerance
 */

void mexFunction (int nlhs, mxArray *phls[], int nrhs, const mxArray *prhs[]) {

  /* Help */
  if (nrhs < 5) {
    mexPrintf ("MsWriteMseed - Gaute Hope <eg@gaute.vetsj.com> / 2012-10-10\n\n");
    mexPrintf ("Usage: \n");
    mexPrintf ("  MsWriteMseed ( batches, dataseries, network, station, location, [channel], [sample rate], [timetol], [sampletol] )\n\n");

    mexPrintf ("  batches is a matrix with a row for each batch with a column for:\n");
    mexPrintf ("    - Start time\n");
    mexPrintf ("    - End time\n");
    mexPrintf ("    - Number of samples\n");
    mexPrintf ("    - Data quality\n");
    mexPrintf ("  dataseries is a vector with data values in same order as batches\n");
    mexPrintf ("  network, station, location and channel (default BNR) specify source\n");
    mexPrintf ("  sample rate, default: 250 Hz\n");
    mexPrintf ("  timetol, tolerance between batches before splitting them in several traces, default: 1.0\n");
    mexPrintf ("  sampletol, tolerance for sample rate before splitting them in several traces, default: 250\n\n");

    mexErrMsgTxt ("Too few arguments.");
  }

  if (nlhs > 0) {
    mexErrMsgTxt ("MsWriteMseed does not support any output arguments.");
  }

  /* Steps:
   *
   * 1. Set up trace group, configure common parameters.
   * 2. Add each batch as record.
   * 3. Write out to file with name defined by source info (SEISAN compatible).
   *
   */

}

