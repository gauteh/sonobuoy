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

  /* Help and argument checking */
  if (nrhs < 5 || nrhs > 9) {
    mexPrintf ("MsWriteMseed - Gaute Hope <eg@gaute.vetsj.com> / 2012-10-10\n\n");
    mexPrintf ("   Create miniSEED files.\n\n");
    mexPrintf ("Usage: \n");
    mexPrintf ("  MsWriteMseed ( batches, dataseries, network, station, location, [channel], [sample rate], [timetol], [sampletol] )\n\n");

    mexPrintf ("  batches is a matrix with a row for each batch with a column for:\n");
    mexPrintf ("    - Start time (hptime_t)\n");
    mexPrintf ("    - End time   (hptime_t)\n");
    mexPrintf ("    - Number of samples\n");
    mexPrintf ("    - Data quality\n");
    mexPrintf ("  dataseries is a vector with data values in same order as batches\n");
    mexPrintf ("  network, station, location and channel (default BNR) specify source\n");
    mexPrintf ("  sample rate, default: 250 Hz\n");
    mexPrintf ("  timetol, tolerance between batches before splitting them in several traces, default: 1.0\n");
    mexPrintf ("  sampletol, tolerance for sample rate before splitting them in several traces, default: 250\n\n");

    mexErrMsgTxt ("Incorrect number of arguments.");
  }

  if (nlhs > 0) {
    mexErrMsgTxt ("MsWriteMseed does not support any output arguments.");
  }

  /* Steps:
   *
   * 0. Get data.
   * 1. Set up trace group, configure common parameters.
   * 2. Add each batch as record.
   * 3. Write out to file with name defined by source info (SEISAN compatible).
   *
   */

  /* Batches matrix */
  const mxArray * batches = prhs[0];

  int batches_n = mxGetN (batches);
  int batches_m = mxGetM (batches);

  if (batches_n != 4) {
    mexErrMsgTxt ("Wrong number of columns in batches.");
  }

  if (batches_m < 1) {
    mexErrMsgTxt ("Empty batches.");
  }

  /* Data series */
  const mxArray * dataseries = prhs[1];
  double * values     = mxGetPr (dataseries);
  int numberofsamples = mxGetN (dataseries);

  if (numberofsamples < 1) {
    mexErrMsgTxt ("No values.");
  }

  /* Network, station, location, channel */
  const mxArray * s = prhs[2]; /* network */
  int s_len         = mxGetN (s) + 1;
  char * network = mxCalloc (s_len, sizeof (char));
  mxGetString (s, network, s_len);
  for (int i = 0; i < s_len; i++) network[i] = toupper(network[i]);

  s     = prhs[3]; /* station */
  s_len = mxGetN (s) + 1;
  char * station = mxCalloc (s_len, sizeof (char));
  mxGetString (s, station, s_len);
  for (int i = 0; i < s_len; i++) station[i] = toupper(station[i]);

  s     = prhs[4]; /* location */
  s_len = mxGetN (s) + 1;
  char * location = mxCalloc (s_len, sizeof (char));
  mxGetString (s, location, s_len);
  for (int i = 0; i < s_len; i++) location[i] = toupper(location[i]);

  char * channel;
  if (nrhs > 5) {
    s     = prhs[5];
    s_len = mxGetN (s) + 1;
    channel = mxCalloc (s_len, sizeof (char));
    mxGetString (s, channel, s_len);
  } else {
    channel = mxCalloc (strlen("BNR0"), sizeof (char));
    strncpy (channel, "BNR", 3); /* default */
  }
  for (int i = 0; i < s_len; i++) channel[i] = toupper(channel[i]);

  /* sample rate */
  double samplerate = 250.0;
  if (nrhs > 6) {
    const mxArray * m = prhs[6];
    samplerate = mxGetScalar (m);
  }

  /* timetol */
  double timetol = 1.0;
  if (nrhs > 7) {
    const mxArray * m = prhs[7];
    timetol = mxGetScalar (m);
  }

  /* sampletol */
  double sampletol = 250.0;
  if (nrhs > 8) {
    const mxArray * m = prhs[8];
    sampletol = mxGetScalar (m);
  }

  mexPrintf ("Writing MiniSEED file for: %s_%s_%s_%s..\n\n", network, station,
      location, channel);

  mexPrintf ("=> Batches:         %4d\n", batches_m);
  mexPrintf ("=> Total samples:   %4d\n", numberofsamples);
  mexPrintf ("=> Sample rate:     %6.1f Hz\n", samplerate);
  mexPrintf ("=> Time tol.:       %6.1f s\n", timetol);
  mexPrintf ("=> Samplerate tol:  %6.1f Hz\n", sampletol);

  mexPrintf ("\n");

}

