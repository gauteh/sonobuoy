/* Author:  Gaute Hope <eg@gaute.vetsj.com>
 * Date:    2012-10-12
 *
 * MATLAB interface to reading DAT files
 *
 */

# include <mex.h>
# include <time.h>

# include "dat.h"

using namespace Zero;
using namespace std;

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

  if (nrhs != 1) {
    mexPrintf ("readdat - Gaute Hope <eg@gaute.vetsj.com> / 2012-10-10\n\n");
    mexPrintf ("   Read dat files.\n\n");
    mexPrintf ("Usage: \n");
    mexPrintf ("  [t, d, refs] = readdat ( filename )\n\n");
  }

  int id = mxGetScalar (prhs[0]);

  mexPrintf ("=> Reading %d..", id);

  Dat dat (id);

  if (!dat.ready) {
    mexPrintf ("failed.\n");
  } else {
    mexPrintf ("read: %d samples.\n", dat.bdata->totalsamples);

    /* Generate refs */
    const int ndim = 10;
    mxArray * refs = mxCreateDoubleMatrix (dat.bdata->batchcount, ndim, mxREAL);
    double  * refs_p = mxGetPr (refs);

    int mdim = dat.bdata->batchcount;

    for (int i = 0; i < dat.bdata->batchcount; i++) {
      refs_p[i + mdim * 0] = (int) 'R';
      refs_p[i + mdim * 1] = (double) dat.bdata->batches[i].length;
      refs_p[i + mdim * 2] = (double) dat.bdata->batches[i].no;
      refs_p[i + mdim * 3] = (double) dat.bdata->batches[i].ref;
      refs_p[i + mdim * 4] = (double) dat.bdata->batches[i].status;

      double t = 0;
      char c = dat.bdata->batches[i].latitude[9];
      char s[15];
      strncpy (s, dat.bdata->batches[i].latitude.c_str(), 9);
      s[9] = '\0';

      int n = 0;
      n = sscanf (s, "%lf", &t);
      if (n > 0) {
        refs_p[i + mdim * 5] = t;
        refs_p[i + mdim * 6] = (int) c;
      }

      t = 0;
      c = 0;
      strncpy (s, dat.bdata->batches[i].longitude.c_str(), 10);
      s[10] = '\0';
      c = dat.bdata->batches[i].longitude[10];
      n = sscanf (s, "%lf", &t);
      if (n > 0) {
        refs_p[i + mdim * 7] = t;
        refs_p[i + mdim * 8] = (int) c;
      }

      refs_p[i + mdim * 9] = (double) dat.bdata->batches[i].checksum;
    }

    /* Generate times */
    mxArray * time = mxCreateDoubleMatrix (dat.bdata->totalsamples, 1, mxREAL);
    double  * time_p = mxGetPr (time);
    int curstart = 0;
    for (int i = 0; i < dat.bdata->batchcount; i++) {
      hptime_t cur_ref = dat.bdata->batches[i].ref;
      time_p[curstart] = (double) cur_ref;

      double delta = 1.0 / SAMPLERATE * 1e6;

      for (int j = 0; j < dat.bdata->batches[i].length; j++) {
        time_p[curstart + j] = cur_ref + j * delta;
      }

      curstart += dat.bdata->batches[i].length;
    }

    /* Build data */
    mxArray * data = mxCreateDoubleMatrix (dat.bdata->totalsamples, 1, mxREAL);
    double  * data_p = mxGetPr(data);

    curstart = 0;
    for (int i = 0; i < dat.bdata->batchcount; i++) {
      for (int j = 0; j < dat.bdata->batches[i].length; j++) {
        data_p[curstart + j] = (double) dat.bdata->batches[i].samples_i[j];
      }
      curstart += dat.bdata->batches[i].length;
    }

    plhs[0] = time;
    plhs[1] = data;
    plhs[2] = refs;
    nlhs = 3;
  }
}


