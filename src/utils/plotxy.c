/*
 * This file is part of MVTH - the Machine Vision Test Harness.
 *
 * Plot a graph of xy data within a viewport.
 *
 * Copyright (C) 2005 Samuel P. Bromley <sam@sambromley.com>
 *
 * This code is licensed under the terms and conditions of
 * the GNU GPL. See the file COPYING in the top level directory of
 * the MVTH archive for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include "base/images_types.h"
#include "drawLine.h"

int plotxy(float *img, int w, int h, int bands,
		double x[], double y[], size_t len, float *rgb,
		ViewPort_t *viewport)
{
	int *ix=NULL, *iy=NULL;
	double xscale,yscale; /* scale from data to image coords */
	double xmin,ymin;
	double xmax,ymax;
	int xlow,ylow;
	int xhigh,yhigh;
	int subregion_width, subregion_height;
	int i;
	if (x==NULL || y==NULL || len==0 || img==NULL || viewport==NULL) return -1;


	xmin=viewport->x_axis.min;
	ymin=viewport->y_axis.min;
	xmax=viewport->x_axis.max;
	ymax=viewport->y_axis.max;
	xlow=(int)(w*viewport->xmin);
	xhigh=(int)(w*viewport->xmax);
	ylow=(int)(h*viewport->ymin);
	yhigh=(int)(h*viewport->ymax);

	if (xlow>=w || xhigh<0 || yhigh<0 || ylow>=h) return -1;
	if (xlow<0) xlow=0;
	if (ylow<0) ylow=0;
	if (xhigh<xlow) return -2;
	if (yhigh<ylow) return -2;
	if (xhigh>w-1) xhigh=w-1;
	if (yhigh>h-1) yhigh=h-1;

	xscale=(xhigh-xlow)/(xmax-xmin);
	yscale=(yhigh-ylow)/(ymax-ymin);

	subregion_width=xhigh-xlow+1;
	subregion_height=yhigh-ylow+1;

	/* allocate memory for the drawPolyline arrays */
	ix=(int*)malloc(len*sizeof(int));
	iy=(int*)malloc(len*sizeof(int));
	for (i=0;i<(int)len;i++)
	{
		int xtmp=(int)((x[i]-xmin)*xscale+0.5);
		int ytmp=(int)((y[i]-ymin)*yscale+0.5);
		/* Lines are clipped later. Don't mess with data here.
		if (xtmp<xlow) xtmp=xlow;
		if (xtmp>xhigh) xtmp=xhigh;
		if (ytmp<ylow) ytmp=ylow;
		if (ytmp>yhigh) ytmp=yhigh;
		*/
		ix[i]=xtmp;
		iy[i]=(subregion_height-1)-ytmp; /* invert y */
	}
	float *subimage=img+bands*(w*(h-1-yhigh)+xlow);
	int pitch=bands*w;
	drawPolyLine(subimage,
			subregion_width, subregion_height,bands,pitch,
			ix,iy,len,rgb);
	free(ix);
	free(iy);

	return 0;
}

int plotxydots(float *img, int w, int h, int bands,
		double x[], double y[], size_t len, float *rgb,
		ViewPort_t *viewport)
{
	int *ix=NULL, *iy=NULL;
	double xscale,yscale; /* scale from data to image coords */
	double xmin,ymin;
	double xmax,ymax;
	int xlow,ylow;
	int xhigh,yhigh;
	int subregion_width, subregion_height;
	int i;
	if (x==NULL || y==NULL || len==0 || img==NULL || viewport==NULL) return -1;

	xmin=viewport->x_axis.min;
	ymin=viewport->y_axis.min;
	xmax=viewport->x_axis.max;
	ymax=viewport->y_axis.max;
	xlow=(int)(w*viewport->xmin);
	xhigh=(int)(w*viewport->xmax);
	ylow=(int)(h*viewport->ymin);
	yhigh=(int)(h*viewport->ymax);

	if (xlow>=w || xhigh<0 || yhigh<0 || ylow>=h) return -1;
	if (xlow<0) xlow=0;
	if (ylow<0) ylow=0;
	if (xhigh<xlow) return -2;
	if (yhigh<ylow) return -2;
	if (xhigh>w-1) xhigh=w-1;
	if (yhigh>h-1) yhigh=h-1;

	xscale=(xhigh-xlow)/(xmax-xmin);
	yscale=(yhigh-ylow)/(ymax-ymin);

	subregion_width=xhigh-xlow+1;
	subregion_height=yhigh-ylow+1;

	/* allocate memory for the drawPolyline arrays */
	ix=(int*)malloc(len*sizeof(int));
	iy=(int*)malloc(len*sizeof(int));
	for (i=0;i<(int)len;i++)
	{
		int xtmp=(int)((x[i]-xmin)*xscale+0.5);
		int ytmp=(int)((y[i]-ymin)*yscale+0.5);
		/*
		if (xtmp<xlow) xtmp=xlow;
		if (xtmp>xhigh) xtmp=xhigh;
		if (ytmp<ylow) ytmp=ylow;
		if (ytmp>yhigh) ytmp=yhigh;
		*/
		ix[i]=xtmp;
		iy[i]=(subregion_height-1)-ytmp; /* invert y */
	}
	float *subimage=img+bands*(w*(h-1-yhigh)+xlow);
	int pitch=bands*w;
	drawDots(subimage,subregion_width,subregion_height,bands,pitch,
			ix,iy,len,rgb);
	free(ix);
	free(iy);

	return 0;
}
