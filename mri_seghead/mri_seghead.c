/**
 * @file  mri_seghead.c
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: REPLACE_WITH_FULL_NAME_OF_CREATING_AUTHOR 
 * CVS Revision Info:
 *    $Author: greve $
 *    $Date: 2014/03/25 16:06:42 $
 *    $Revision: 1.8 $
 *
 * Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
 *
 * Terms and conditions for use, reproduction, distribution and contribution
 * are found in the 'FreeSurfer Software License Agreement' contained
 * in the file 'LICENSE' found in the FreeSurfer distribution, and here:
 *
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
 *
 * Reporting: freesurfer@nmr.mgh.harvard.edu
 *
 */


/*
  Name:    mri_seghead.c
  Author:  Douglas N. Greve
  email:   analysis-bugs@nmr.mgh.harvard.edu
  Date:    2/27/02
  Purpose: Segment the head.
  $Id: mri_seghead.c,v 1.8 2014/03/25 16:06:42 greve Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#include "error.h"
#include "diag.h"
#include "proto.h"
#include "mrisurf.h"

#include "fio.h"
#include "mri.h"
#include "MRIio_old.h"
#include "cmdargs.h"


static int  parse_commandline(int argc, char **argv);
static void check_options(void);
static void print_usage(void) ;
static void usage_exit(void);
static void print_help(void) ;
static void print_version(void) ;
static void argnerr(char *option, int n);
static void dump_options(FILE *fp);
static int  isflag(char *flag);
static int  singledash(char *flag);
static int  stringmatch(char *str1, char *str2);
int main(int argc, char *argv[]) ;

static char vcid[] = "$Id: mri_seghead.c,v 1.8 2014/03/25 16:06:42 greve Exp $";
char *Progname = NULL;
int debug = 0;
char *subjid;

char *involid;
char *outvolid;
unsigned char fillval = 255;
int thresh1  = -1;
int thresh2  = -1;
int nhitsmin = 2;

int fillslices = 1;
int fillrows = 1;
int fillcols = 1;

int dilate = 0;

MRI *invol;
MRI *HitMap;
MRI *outvol, *outvol2;
MRI *cvol, *rvol, *svol;
MRI *mrgvol;
MRI *invol_orig;

char tmpstr[1000];
char *hvoldat = NULL;
char *SUBJECTS_DIR;
FILE *fp;

int MRISprojectDist(MRIS *surf, const MRI *mridist);
int MRISbrainSurfToSkull(MRIS *surf, const double *params, const MRI *vol);
int MakeSkullSurface(char *subject, double *params, char *innername, char *outername);

/*---------------------------------------------------------------*/
int main(int argc, char **argv) {
  int c,r,s,n;
  int smin, smax, rmin, rmax, cmin, cmax, nhits;

  /* This is to shut up the compiler */
  isflag(" ");

  Progname = argv[0] ;
  argc --;
  argv++;
  ErrorInit(NULL, NULL, NULL) ;
  DiagInit(NULL, NULL, NULL) ;

  if (argc == 0) usage_exit();

  parse_commandline(argc, argv);
  check_options();

  dump_options(stdout);

  /* Make sure output directory is writable */
  if (! fio_DirIsWritable(outvolid,1) &&
      fio_DirIsWritable(outvolid,0)) {
    printf("ERROR: could not access output %s\n",outvolid);
    exit(1);
  }

  printf("Loading input volume\n");
  fflush(stdout);
  invol_orig = MRIread(involid);
  if (invol_orig == NULL) {
    printf("ERROR: could not read %s\n",involid);
    exit(1);
  }

  invol = MRIchangeType(invol_orig,MRI_UCHAR,0,255,1);
  if (invol == NULL) {
    printf("ERROR: bvolumeWrite: MRIchangeType\n");
    return(1);
  }

  cvol = MRIclone(invol,NULL);
  rvol = MRIclone(invol,NULL);
  svol = MRIclone(invol,NULL);
  mrgvol = MRIclone(invol,NULL);
  outvol = MRIclone(invol,NULL);

  if (fillcols) {
    printf("Filling Columns\n");
    fflush(stdout);
    for (r=0; r < invol->height; r++) {
      for (s=0; s < invol->depth; s++) {

        c = 0;
        nhits = 0;
        while (c < invol->width && nhits < nhitsmin) {
          if (MRIseq_vox(invol,c,r,s,0) > thresh1) nhits ++;
          else nhits = 0;
          c++;
        }
        if (nhits < nhitsmin) continue;
        cmin = c;

        c = invol->width - 1;
        nhits = 0;
        while (c > cmin && nhits < nhitsmin) {
          if (MRIseq_vox(invol,c,r,s,0) > thresh1) nhits ++;
          else nhits = 0;
          c--;
        }
        cmax = c;

        if (cmin >= dilate) cmin-=dilate;
        else cmin = 0;
        if (cmax <= invol->width-1-dilate) cmax+=dilate;
        else cmax = invol->width-1; 

        for (c = cmin; c <= cmax; c++) MRIseq_vox(cvol,c,r,s,0) = fillval;
        //printf("%3d %3d  %3d %3d\n",r,s,cmin,cmax);
      }
    }
  }

  if (fillrows) {
    printf("Filling Rows\n");
    fflush(stdout);
    for (c=0; c < invol->width; c++) {
      for (s=0; s < invol->depth; s++) {

        r = 0;
        nhits = 0;
        while (r < invol->height && nhits < nhitsmin) {
          if (MRIseq_vox(invol,c,r,s,0) > thresh1) nhits ++;
          else nhits = 0;
          r++;
        }
        if (nhits < nhitsmin) continue;
        rmin = r;

        r = invol->height - 1;
        nhits = 0;
        while (r > rmin && nhits < nhitsmin) {
          if (MRIseq_vox(invol,c,r,s,0) > thresh1) nhits ++;
          else nhits = 0;
          r--;
        }
        rmax = r;

        if (rmin >= dilate) rmin-=dilate;
        else rmin = 0;
        if (rmax <= invol->height-1-dilate) rmax+=dilate;
        else rmax = invol->height-1; 
        
        for (r = rmin; r <= rmax; r++) MRIseq_vox(rvol,c,r,s,0) = fillval;
        //printf("%3d %3d  %3d %3d\n",r,s,rmin,rmax);
      }
    }
  }

  if (fillslices) {
    printf("Filling Slices\n");
    fflush(stdout);
    for (c=0; c < invol->width; c++) {
      for (r=0; r < invol->height; r++) {

        s = 0;
        nhits = 0;
        while (s < invol->depth && nhits < nhitsmin) {
          if (MRIseq_vox(invol,c,r,s,0) > thresh1) nhits ++;
          else nhits = 0;
          s++;
        }
        if (nhits < nhitsmin) continue;
        smin = s;

        s = invol->depth - 1;
        nhits = 0;
        while (s > smin && nhits < nhitsmin) {
          if (MRIseq_vox(invol,c,r,s,0) > thresh1) nhits ++;
          else nhits = 0;
          s--;
        }
        smax = s;

        if (smin >= dilate) smin-=dilate;
        else smin = 0;
        if (smax <= invol->depth-1-dilate) smax+=dilate;
        else smax = invol->depth-1; 
        
        for (s = smin; s <= smax; s++) MRIseq_vox(svol,c,r,s,0) = fillval;
        //printf("%3d %3d  %3d %3d\n",r,s,rmin,rmax);
      }
    }
  }

  printf("Merging and Inverting\n");
  for (c=0; c < invol->width; c++) {
    for (r=0; r < invol->height; r++) {
      for (s=0; s < invol->depth; s++) {
        if (MRIseq_vox(cvol,c,r,s,0) &&
            MRIseq_vox(rvol,c,r,s,0) &&
            MRIseq_vox(svol,c,r,s,0))
          MRIseq_vox(mrgvol,c,r,s,0) = 0;
        else
          MRIseq_vox(mrgvol,c,r,s,0) = 255-MRIseq_vox(invol,c,r,s,0);
      }
    }
  }
  
  printf("Growing\n");
  fflush(stdout);
  outvol = MRIfill(mrgvol, NULL, (int)(mrgvol->width/2.0),
                   (int)(mrgvol->height/2.0), (int)(mrgvol->depth/2.),
                   thresh2, fillval, -1);

  printf("Counting\n");
  n = 0;
  double backnoise = 0.0;
  int backcount = 0;
  double backmax = 0.0;
  double val = 0.0;
  for (c=0; c < invol->width; c++) {
    for (r=0; r < invol->height; r++) {
      for (s=0; s < invol->depth; s++) {
        if (MRIseq_vox(outvol,c,r,s,0)) n++;
        else
        {
          val = MRIgetVoxVal(invol_orig,c,r,s,0);
          backnoise += val;
          if (backmax < val) backmax = val;
          backcount++;
        }
      }
    }
  }
  backnoise= backnoise/backcount;
  printf("N Head Voxels = %d\n",n);
  printf("N Back Voxels = %d\n",backcount);
  printf("Avg. Back Intensity = %f\n",backnoise);
  printf("Max. Back Intensity = %f\n",backmax);
  
  if(hvoldat){
    fp = fopen(hvoldat,"w");
    fprintf(fp,"%lf\n",n*invol->xsize*invol->ysize*invol->zsize);
    fclose(fp);
  }

  if(outvolid){
    printf("Writing output\n");
    fflush(stdout);
    MRIwrite(outvol,outvolid);
  }

  printf("Done\n");

  return(0);
}
/* ------------------------------------------------------------------ */
static int parse_commandline(int argc, char **argv) {
  int  nargc , nargsused;
  char **pargv, *option ;
  int a;

  if (argc < 1) usage_exit();

  nargc   = argc;
  pargv = argv;
  while (nargc > 0) {

    option = pargv[0];
    if (debug) printf("%d %s\n",nargc,option);

    nargc -= 1;
    pargv += 1;
    nargsused = 0;

    if (!strcasecmp(option, "--help"))  print_help() ;
    else if (!strcasecmp(option, "--version")) print_version() ;
    else if (!strcasecmp(option, "--debug"))   debug = 1;
    else if (!strcasecmp(option, "--fillrows")) fillrows = 1;
    else if (!strcasecmp(option, "--fillcols")) fillcols = 1;
    else if (!strcasecmp(option, "--fillslices")) fillslices = 1;

    else if ( stringmatch(option, "--invol") ||
              stringmatch(option, "--i") ) {
      if (nargc < 1) argnerr(option,1);
      involid = pargv[0];
      nargsused = 1;
    } else if (stringmatch(option, "--outvol") ||
               stringmatch(option, "--o") ) {
      if (nargc < 1) argnerr(option,1);
      outvolid = pargv[0];
      nargsused = 1;
    } else if ( stringmatch(option, "--subject") ||
                stringmatch(option, "--s") ) {
      if (nargc < 1) argnerr(option,1);
      subjid = pargv[0];
      nargsused = 1;
    } else if (stringmatch(option, "--fill")) {
      if (nargc < 1) argnerr(option,1);
      sscanf(pargv[0],"%d",&a);
      fillval = (unsigned char) a;
      nargsused = 1;
    } else if (stringmatch(option, "--thresh1")) {
      if (nargc < 1) argnerr(option,1);
      sscanf(pargv[0],"%d",&thresh1);
      nargsused = 1;
    } else if (stringmatch(option, "--thresh2")) {
      if (nargc < 1) argnerr(option,1);
      sscanf(pargv[0],"%d",&thresh2);
      nargsused = 1;
    } else if (stringmatch(option, "--thresh")) {
      if (nargc < 1) argnerr(option,1);
      sscanf(pargv[0],"%d",&thresh1);
      thresh2 = thresh1;
      nargsused = 1;
    } else if (stringmatch(option, "--nhitsmin")) {
      if (nargc < 1) argnerr(option,1);
      sscanf(pargv[0],"%d",&nhitsmin);
      nargsused = 1;
    } else if (stringmatch(option, "--hvoldat")) {
      if (nargc < 1) argnerr(option,1);
      hvoldat = pargv[0];
      nargsused = 1;
    } 
    else if (stringmatch(option, "--dilate")) {
      if (nargc < 1) argnerr(option,1);
      sscanf(pargv[0],"%d",&dilate);
      nargsused = 1;
    } 
    else if (stringmatch(option, "--skull")) {
      char *subject, *innername, *outername; 
      double params[7];
      if(nargc < 3) argnerr(option,3);
      subject = pargv[0];
      innername = pargv[1]; // relative to subject/surf
      outername = pargv[2]; // relative to subject/surf
      params[0] = .5; // athresh
      params[1] = 2; //minthick
      params[2] = 9; //maxthick
      params[3] = 30; //maxdist
      params[4] = .5; //stepsize
      params[5] = 2; //ndils
      params[6] = 10; //navgs
      MakeSkullSurface(subject, params, innername, outername);
      exit(0);
      nargsused = 3;
    } 
    else if (!strcmp(option, "--sd") || !strcmp(option, "-SDIR")) {
      if(nargc < 1) CMDargNErr(option,1);
      setenv("SUBJECTS_DIR",pargv[0],1);
      SUBJECTS_DIR = getenv("SUBJECTS_DIR");
      nargsused = 1;
    } 
    else {
      fprintf(stderr,"ERROR: Option %s unknown\n",option);
      if (singledash(option))
        fprintf(stderr,"       Did you really mean -%s ?\n",option);
      exit(-1);
    }
    nargc -= nargsused;
    pargv += nargsused;
  }
  return(0);
}
/* ------------------------------------------------------ */
static void usage_exit(void) {
  print_usage() ;
  exit(1) ;
}
/* --------------------------------------------- */
static void print_usage(void) {
  printf("USAGE: %s \n",Progname) ;
  printf("\n");
  printf("   --invol     input volume id (eg, T1)\n");
  printf("   --outvol    input volume id\n");
  printf("   --fill      fill value  (255)\n");
  printf("   --thresh1   threshold value  (eg, 20)\n");
  printf("   --thresh2   threshold value  (eg, 20)\n");
  printf("   --thresh    single threshold value for 1 and 2 \n");
  printf("   --nhitsmin  min number of consecutive hits (2) \n");
  printf("   --hvoldat   file : write head volume (mm3) to an ascii file \n");
  printf("\n");
}
/* --------------------------------------------- */
static void print_help(void) {
  print_usage() ;

  printf("\n%s\n\n",vcid);

  printf(
    "This program binarizes the input volume such that all the voxels in the \n"
    "head are set to 255 (or whatever is passed with --fill). The result is \n"
    "stored in the output volume passed by --outvol. \n"
    " \n"
    "The program first creates a binarized mass just a few mm below the skin.\n"
    "This mass is then grown out using a connected components algorithm so \n"
    "that most of the skin details are retained. \n"
    " \n"
    "The initial mass is created in the following way. First, for every row \n"
    "and slice, the column is searched from both ends for the 'skin'. The \n"
    "skin is defined as the first consecutive nhitsmin voxels over thresh1.\n"
    "Once the skin is found coming from both directions, everything in between\n"
    "is binarized to the fill value. This process is repeated for the rows \n"
    "and slices. The initial mass is created by ANDing all three voxels.\n"
    " \n"
    "After the initial mass is defined, the original volume is modified so\n"
    "that all the voxels in the mass are set to 255. This has the effect of \n"
    "filling in all of the subepidermal features that would normally be below \n"
    "threshold. A seed point is chosen at the center of the volume. The final \n"
    "binarized volume is computed as all the voxels above thresh2 connected to  \n"
    "the seed point. \n"
    " \n"
    "--thresh threshold will set thresh1 and thresh2 to threshold. \n"
    " \n"
    "Typical values for the parameters are: thresh=20 and nhitsmin=2. \n"
    " \n"
  );

  exit(1) ;
}
/* --------------------------------------------- */
static void print_version(void) {
  printf("%s\n", vcid) ;
  exit(1) ;
}
/* --------------------------------------------- */
static void argnerr(char *option, int n) {
  if (n==1)
    fprintf(stderr,"ERROR: %s flag needs %d argument\n",option,n);
  else
    fprintf(stderr,"ERROR: %s flag needs %d arguments\n",option,n);
  exit(-1);
}
/* --------------------------------------------- */
static void check_options(void) {
  if (involid == NULL) {
    printf("An input volume path must be supplied\n");
    exit(1);
  }
/*  if(outvolid == NULL && hvoldat == NULL) {
    printf("An output volume path must be supplied\n");
    exit(1);
  }*/
  if (thresh1 <= 0) {
    printf("Must specify a thresh1 > 0\n");
    exit(1);
  }
  if (thresh2 <= 0) {
    printf("Must specify a thresh2 > 0\n");
    exit(1);
  }
  if (nhitsmin <= 0) {
    printf("Must specify a nhitsmin > 0\n");
    exit(1);
  }
  return;
}
/* --------------------------------------------- */
static void dump_options(FILE *fp) {
  fprintf(fp,"input volume:  %s\n",involid);
  fprintf(fp,"output volume: %s\n",outvolid);
  fprintf(fp,"threshold1:    %d\n",thresh1);
  fprintf(fp,"threshold2:    %d\n",thresh2);
  fprintf(fp,"nhitsmin:      %d\n",nhitsmin);
  fprintf(fp,"fill value:    %d\n",(int)fillval);
  if (dilate > 0) fprintf(fp,"dilate    :    %d\n",dilate);
  return;
}
/*---------------------------------------------------------------*/
static int singledash(char *flag) {
  int len;
  len = strlen(flag);
  if (len < 2) return(0);
  if (flag[0] == '-' && flag[1] != '-') return(1);
  return(0);
}
/*---------------------------------------------------------------*/
static int isflag(char *flag) {
  int len;
  len = strlen(flag);
  if (len < 2) return(0);
  if (flag[0] == '-' && flag[1] == '-') return(1);
  return(0);
}
/*------------------------------------------------------------*/
static int stringmatch(char *str1, char *str2) {
  if (! strcmp(str1,str2)) return(1);
  return(0);
}


int MakeSkullSurface(char *subject, double *params, char *innername, char *outername)
{
  char *SUBJECTS_DIR, tmpstr[5000];
  MRIS *surf,*surf2;
  MRI *mri;
  int ndils,navgs,n,err;
  //athresh  = params[0]; // .5
  //minthick = params[1]; // 2
  //maxthick = params[2]; // 9
  //maxdist  = params[3]; // 30
  //stepsize = params[4]; // 0.5
  ndils = nint(params[5]);
  navgs = nint(params[6]);

  SUBJECTS_DIR = getenv("SUBJECTS_DIR");
  sprintf(tmpstr,"%s/%s/surf/%s",SUBJECTS_DIR,subject,innername);
  printf("Reading brain surface %s\n",tmpstr);
  surf = MRISread(tmpstr);
  if(surf == NULL){
    printf("ERROR: cannot find %s\n",tmpstr);
    exit(1);
  }
  sprintf(tmpstr,"%s/%s/mri/nu.mgz",SUBJECTS_DIR,subject);
  printf("Reading volume %s\n",tmpstr);
  mri = MRIread(tmpstr);
  if(mri == NULL) exit(1);

  printf("Projecting inner surface to outer skull\n");
  MRISbrainSurfToSkull(surf, params, mri);

  printf("Filling skull interior\n");
  MRISfillInterior(surf, 0, mri) ; // replaces values in mri

  printf("Dilating by %d\n",ndils);
  for(n=0; n<ndils; n++) MRIdilate(mri,mri);

  printf("Eroding by %d\n",ndils);
  for(n=0; n<ndils; n++) MRIerode(mri,mri);

  printf("Retessellating skull surface\n");
  surf2 = MRIStessellate(mri, 1, 0);

  printf("Smoothiing vertex positions niters = %d \n",navgs);
  MRISaverageVertexPositions(surf2, navgs) ;

  sprintf(tmpstr,"%s/%s/surf/%s",SUBJECTS_DIR,subject,outername);
  printf("Writing to %s\n",tmpstr);
  err = MRISwrite(surf2,tmpstr);
  if(err){
    printf("ERROR: writing %s\n",tmpstr);
    exit(1);
  }

  MRIfree(&mri);
  MRISfree(&surf);
  MRISfree(&surf2);
  return(0);
}

/*
  \fn int MRISprojectDist(MRIS *surf, const MRI *mridist)
  \brief Project surface along the normal a distance given by the value in mridist
  at the voxel corresponding to the vertex.
 */
int MRISprojectDist(MRIS *surf, const MRI *mridist)
{
  VERTEX *vtx;
  int vno,nthstep;
  double dist;

  for(vno = 0; vno < surf->nvertices; vno++){
    vtx = &(surf->vertices[vno]);
    for(nthstep = 0; nthstep < mridist->nframes; nthstep++){
      dist = MRIgetVoxVal(mridist,vno,0,0,nthstep);
      vtx->x += dist*vtx->nx;
      vtx->y += dist*vtx->ny;
      vtx->z += dist*vtx->nz;
    }
  }
  return(0);
}

/*
  \fn int MRISbrainSurfToSkull(MRIS *surf, const double *params, const MRI *vol)
  \brief This is a slight hack to try to find the outer surface of the
  skull. The surf should be something like the brain surface
  (including xcsf). This function then projects along the normal for
  maxdist mm sampling vol every stepsize and finds the maximum
  (expected to be skin), then steps back until the signal intensity
  (wrt max-min) drops by a fraction of athresh. The skull thickness is
  constrained to be between minthick and maxthick. vol should probably
  be nu.mgz. params: 0=athresh, 1=minthick, 2=maxthick, 3=maxdist,
  4=stepsize.  All distances are in mm. This is a very crude method
  and it is not expected that it will always yield an accurate skull.
  The surface could be refined by filling it in, dilating, eroding,
  tessellating, smoothing.
 */
int MRISbrainSurfToSkull(MRIS *surf, const double *params, const MRI *vol)
{
  VERTEX *vtx;
  MRI *mridist;
  int vno,c,r,s,nthstep,nthstepvmax,nhits,nsteps;
  MATRIX *vox2tkras,*tkras2vox,*xyz,*crs;
  double a,dist,dx,dy,dz,v,vmax,vmin,*vlist,dmax;
  double athresh, maxdist=30, stepsize=0.5, minthick=2, maxthick=9;

  athresh  = params[0]; // .5
  minthick = params[1]; // 2
  maxthick = params[2]; // 9
  maxdist  = params[3]; // 30
  stepsize = params[4]; // 0.5

  vox2tkras = MRIxfmCRS2XYZtkreg(vol);
  tkras2vox = MatrixInverse(vox2tkras,NULL);
  crs = MatrixAlloc(4,1,MATRIX_REAL);
  xyz = MatrixAlloc(4,1,MATRIX_REAL);
  crs->rptr[4][1] = 1;
  xyz->rptr[4][1] = 1;

  nsteps = ceil(maxdist/stepsize)+1;
  vlist = (double *)calloc(sizeof(double),nsteps);

  mridist = MRIallocSequence(surf->nvertices,1,1,MRI_FLOAT,1);

  nhits = 0;
  for(vno = 0; vno < surf->nvertices; vno++){
    vtx = &(surf->vertices[vno]);
    vmax = 0;
    vmin = 1e10;
    nthstepvmax = 0;
    for(nthstep = 0; nthstep < nsteps; nthstep++) vlist[nthstep] = 0;
    // project out along the normal
    for(nthstep = 0; nthstep < nsteps; nthstep++){
      dist = stepsize*nthstep;
      dx = dist*vtx->nx;
      dy = dist*vtx->ny;
      dz = dist*vtx->nz;
      xyz->rptr[1][1] = vtx->x + dx;
      xyz->rptr[2][1] = vtx->y + dy;
      xyz->rptr[3][1] = vtx->z + dz;
      crs = MatrixMultiply(tkras2vox,xyz,crs);
      c = crs->rptr[1][1];
      r = crs->rptr[2][1];
      s = crs->rptr[3][1];
      if(c < 0 || c >= vol->width || 
	 r < 0 || r >= vol->height ||
	 s < 0 || s >= vol->depth){
	continue;
      }
      v = MRIgetVoxVal(vol,c,r,s,0);
      vlist[nthstep] = v;
      if(vmax < v){
	// keep track of where the max intensity is
	vmax = v;
	dmax = dist;
	nthstepvmax = nthstep;
      }
    }

    // Find the minimum between step=0 and where the maximum is
    // Ie, don't find a minimum beyond the max
    vmin = 1e10;
    for(nthstep = 0; nthstep < nthstepvmax; nthstep++)  {
      v = vlist[nthstep];
      if(vmin > v) vmin = v;
    }

    // Start at the maximum and step backwards
    for(nthstep = nthstepvmax; nthstep >= 0; nthstep--){
      // how far has the signal dropped since the maximum?
      a = (vlist[nthstep]-vmin)/(vmax-vmin); 
      if(a < athresh){
	dist = stepsize*nthstep;
	if(dist > maxthick) dist = maxthick;
	if(dist < minthick) dist = minthick;
	MRIsetVoxVal(mridist,vno,0,0,0,dist);
	break;
      }
    }
  } // vertex

  // Move the surface out to the maximum
  MRISprojectDist(surf, mridist);

  MRIfree(&mridist);
  MatrixFree(&vox2tkras);
  MatrixFree(&tkras2vox);
  MatrixFree(&crs);
  MatrixFree(&xyz);
  free(vlist);
  return(0);
}
