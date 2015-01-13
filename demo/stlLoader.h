#pragma once
/***************************************************************************
tribarrier.cpp  -  description
-------------------
begin                : Mit Sep 25 13:11:41 CEST 2002
copyright            : (C) 2002 by Harald Krippel
email                : harald@hte-develop.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include <QtGui>
#include <tribarrier.hpp>
#include <stdio.h>
#include <ode/ode.h>

TriBarrier::TriBarrier(void) {
	objtrans = NULL;
	boxobjtrans = NULL;
	VertexCount = 0;
	IndexCount = 0;
}

void TriBarrier::createTriMesh(dSpaceID odspace, const char *file) {
	data = dGeomTriMeshDataCreate();
	buildTriMeshData(file);
	TriMesh = dCreateTriMesh(odspace, data, 0, 0, 0);
}

void TriBarrier::buildTriMeshData(const char *file) {
	long i, err;
	unsigned int dummy;

	VertexCount = 0;
	IndexCount = 0;
	err = 0;
	i = 0;
#ifdef Q_OS_UNIX
	// set locale ode problem
	char * ctype = qstrdup(setlocale(LC_NUMERIC, 0));
	setlocale(LC_NUMERIC, "C");        // make sprintf()/scanf() work
#endif
	QFile filedata(file);
	if ( filedata.open(QFile::ReadOnly) ) {
		QTextStream in(&filedata);
		do {
			in >> Vertices[i][0] >> Vertices[i][1] >> Vertices[i][2];
			in >> Vertices[i + 1][0] >> Vertices[i + 1][1] >> Vertices[i + 1][2];
			in >> Vertices[i + 2][0] >> Vertices[i + 2][1] >> Vertices[i + 2][2];
			in >> dummy;

			Indices[IndexCount] = IndexCount; IndexCount++;
			Indices[IndexCount] = IndexCount; IndexCount++;
			Indices[IndexCount] = IndexCount; IndexCount++;
			Indices[IndexCount] = IndexCount; IndexCount++;
			Indices[IndexCount] = IndexCount; IndexCount++;
			Indices[IndexCount] = IndexCount; IndexCount++;
			Indices[IndexCount] = IndexCount; IndexCount++;
			Indices[IndexCount] = IndexCount; IndexCount++;
			Indices[IndexCount] = IndexCount; IndexCount++;
			i = i + 3;
			if ( IndexCount >= (MAXINDEX - 9) ) {
				fprintf(stderr, "Tribarrier MaxVertices reached !!\n");
				break;
			}
		} while ( !in.atEnd() && i < MAXVERTEX - 3 );
		VertexCount = i;
		//    Print(); // Debug
	}
	else {
		qDebug() << file << " : File Open Error !!\n";
	}
#ifdef dDOUBLE
	dGeomTriMeshDataBuildSimple(data, (dReal *)Vertices, VertexCount, Indices, IndexCount);
#endif
#ifdef dSINGLE
	//ODE_API void dGeomTriMeshDataBuildSingle(dTriMeshDataID g,
	//                                  const void* Vertices, int VertexStride, int VertexCount, 
	//                                  const void* Indices, int IndexCount, int TriStride);
	dGeomTriMeshDataBuildSingle(data, (float *)&Vertices[0], 3 * sizeof(float), VertexCount, (int*)&Indices[0], IndexCount, 3 * sizeof(int));

#endif
#ifdef Q_OS_UNIX
	// restore locale ode problem
	setlocale(LC_NUMERIC, ctype);
	delete[] ctype;
#endif
}

void TriBarrier::setScale(const float sx, const float sy, const float sz) {
	long a;

	for ( a = 0; a<VertexCount; a++ ) {
		VerticesScale[a][0] = Vertices[a][0] * sx;
		VerticesScale[a][1] = Vertices[a][1] * sy;
		VerticesScale[a][2] = Vertices[a][2] * sz;
	}
#ifdef dDOUBLE
	dGeomTriMeshDataBuildSimple(data, (dReal *)VerticesScale, VertexCount, Indices, IndexCount);
#endif
#ifdef dSINGLE
	dGeomTriMeshDataBuildSingle(data, (float *)&VerticesScale[0], 3 * sizeof(float), VertexCount, (int*)&Indices[0], IndexCount, 3 * sizeof(int));
#endif
	dGeomTriMeshSetData(TriMesh, data);
}

void TriBarrier::Print() {
	long a;
	fprintf(stderr, "VertexCount=%ld\n", VertexCount);
	fprintf(stderr, "IndexCount=%ld\n", IndexCount);
	for ( a = 0; a<VertexCount; a++ ) {
		fprintf(stderr, "Vertex %ld %f %f %f\n", a, Vertices[a][0], Vertices[a][1], Vertices[a][2]);
	}
	for ( a = 0; a<IndexCount; a++ ) {
		fprintf(stderr, "Index %ld %d\n", a, Indices[a]);
	}
}

void TriBarrier::update() {
	//  int a=0;
	//  dVector3 dxyz;

	// positon TriBarrier from ode
	if ( objtrans != NULL ) {
		//      dGeomBoxGetLengths (odbox[a], dxyz);
		//      setTransform(objtrans[a],dGeomGetPosition(odbox[a]),dGeomGetRotation(odbox[a]),dxyz);
	}
}

void TriBarrier::setTransform(ssgTransform *objtrans, const dReal barpos[3], const dReal R[12], dVector3 dxyz) {
	sgMat4 m;

	m[0][0] = R[0];
	m[0][1] = R[4];
	m[0][2] = R[8];
	m[0][3] = SG_ZERO; //x

	m[1][0] = R[1];
	m[1][1] = R[5];
	m[1][2] = R[9];
	m[1][3] = SG_ZERO;  // y

	m[2][0] = R[2];
	m[2][1] = R[6];
	m[2][2] = R[10];
	m[2][3] = SG_ZERO;  // z

	m[3][0] = barpos[0];
	m[3][1] = barpos[1];
	m[3][2] = barpos[2];
	m[3][3] = SG_ONE;

	sgScaleVec3(m[0], dxyz[0]);
	sgScaleVec3(m[1], dxyz[1]);
	sgScaleVec3(m[2], dxyz[2]);

	objtrans->setTransform(m);
	sgSetCoord(&pos, m);
}

void TriBarrier::GetPosition(sgCoord  *bodypos) {
	sgCopyCoord(bodypos, &pos);
}

void TriBarrier::SetPosition() {
	dMatrix3 odR;

	dGeomSetPosition(TriMesh, pos.xyz[0], pos.xyz[1], pos.xyz[2]);
	dRFromEulerAngles(odR, -pos.hpr[0] * SG_DEGREES_TO_RADIANS
					  , -pos.hpr[1] * SG_DEGREES_TO_RADIANS
					  , -pos.hpr[2] * SG_DEGREES_TO_RADIANS);
	dGeomSetRotation(TriMesh, odR);
}

TriBarrier::~TriBarrier() {

}

