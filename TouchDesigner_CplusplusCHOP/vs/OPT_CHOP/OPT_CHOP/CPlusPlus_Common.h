/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with 
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 */

/*
 * Produced by:
 *
 * 				Derivative Inc
 *				401 Richmond Street West, Unit 386
 *				Toronto, Ontario
 *				Canada   M5V 3A8
 *				416-591-3555
 *
 * NAME:				CPlusPlus_Common.h 
 *
 */

/*******
	Derivative Developers:: Make sure the virtual function order
	stays the same, otherwise changes won't be backwards compatible
********/


#ifndef __CPlusPlus_Common
#define __CPlusPlus_Common

#define NOMINMAX

#include <limits>
#include <windows.h>
#include <gl/gl.h>
#include <cstdio>

#define DLLEXPORT __declspec (dllexport)

class OP_NodeInfo
{
public:
	// The full path to the operator

	const char*		opPath;

	// A unique ID representing the operator, no two operators will ever
	// have the same ID in a single Touch instance.

	unsigned int	opID;

	// This is the handle to the main TouchDesigner window.
	// It's possible this will be 0 the first few times the operator cooks,
	// incase it cooks while Touch is still loading up

	HWND			mainWindowHandle;

private:
	int				reserved[19];
};


class OP_DATInput
{
public:
	const char*		opPath;
	unsigned int	opId;

	int             numRows;
	int             numCols;
	bool            isTable;

	// data, referenced by (row,col), which will be a const char* for the
	// contents of the cell
	// E.g getCell(1,2) will be the contents of the cell located at (1,2)

	const char*		getCell(int row, int col) const
					{
						return cellData[row * numCols + col];
					}

protected:
	const char**	cellData;

private:
	int             reserved[20];
};

class OP_TOPInput
{
public:
	const char*		opPath;
	unsigned int	opId;

	int				width;
	int				height;


	// The OpenGL Texture index for this TOP.
	// This is only valid when accessed from C++ TOPs.
	// Other C++ OPs will have this value set to 0 (invalid).
	GLuint			textureIndex;

	// The OpenGL Texture target for this TOP.
	// E.g GL_TEXTURE_2D, GL_TEXTURE_CUBE,
	// GL_TEXTURE_2D_ARRAY
	GLenum			textureType;

	// Depth for 3D and 2D_ARRAY textures, undefined
	// for other texture types
	unsigned int	depth;

	// contains the internalFormat for the texture
	// such as GL_RGBA8, GL_RGBA32F, GL_R16
	GLint			pixelFormat;

private:
	int				reserved[19];
};


class OP_CHOPInput
{
public:

	const char*		opPath;
	unsigned int	opId;

	int				numChannels;
	int				numSamples;
	double			sampleRate;
	double			startIndex;



	// Retrieve a float array for a specific channel.
	// 'i' ranges from 0 to numChannels-1
	// The returned arrray contains 'numSamples' samples.
	// e.g: getChannelData(1)[10] will refer to the 11th sample in the 2nd channel 

	const float*	getChannelData(int i) const
					{
						return channelData[i];
					}


	// Retrieve the name of a specific channel.
	// 'i' ranges from 0 to numChannels-1
	// For example getChannelName(1) is the name of the 2nd channel

	const char*		getChannelName(int i) const
					{
						return nameData[i];
					}

protected:

	const float**	channelData;
	const char**	nameData;


private:

	int				reserved[20];
};

class OP_ObjectInput
{
public:

	const char*		opPath;
	unsigned int	opId;

	// Use these methods to calculate object transforms
	double			worldTransform[4][4];
	double			localTransform[4][4];


private:

	int				reserved[20];
};

class OP_Inputs
{
public:
	// NOTE: When writting a TOP, none of these functions should
	// be called inside a beginGLCommands()/endGLCommands() section
	// as they may require GL themselves to complete execution.

	// these are wired into the node
	virtual int			getNumInputs() = 0;

	// may return nullptr when invalid input
	virtual const OP_TOPInput*		getInputTOP(int index) = 0;	// only valid for TOP operators
	virtual const OP_CHOPInput*		getInputCHOP(int index) = 0;	// only valid for CHOP operators

	// these are defined by parameters.
	// may return nullptr when invalid input
	// this value is valid until the parameters are rebuilt or it is called with the same parameter name.
	virtual const OP_DATInput*		getParDAT(const char *name) = 0;
	virtual const OP_TOPInput*		getParTOP(const char *name) = 0;
	virtual const OP_CHOPInput*		getParCHOP(const char *name) = 0;
	virtual const OP_ObjectInput*	getParObject(const char *name) = 0;

	// these work on any type of parameter and can be interchanged
	// for menu types, int returns the menu selection index, string returns the item

	// returns the requested value, index may be 0 to 4.
	virtual double		getParDouble(const char* name, int index=0) = 0;

	// for multiple values: returns True on success/false otherwise
	virtual bool        getParDouble2(const char* name, double &v0, double &v1) = 0;
	virtual bool        getParDouble3(const char* name, double &v0, double &v1, double &v2) = 0;
	virtual bool        getParDouble4(const char* name, double &v0, double &v1, double &v2, double &v3) = 0;


	// returns the requested value
	virtual int			getParInt(const char* name, int index=0) = 0;

	// for multiple values: returns True on success/false otherwise
	virtual bool        getParInt2(const char* name, int &v0, int &v1) = 0;
	virtual bool        getParInt3(const char* name, int &v0, int &v1, int &v2) = 0;
	virtual bool        getParInt4(const char* name, int &v0, int &v1, int &v2, int &v3) = 0;

	// returns the requested value
	// this value is valid until the parameters are rebuilt or it is called with the same parameter name.
	virtual const char*	getParString(const char* name) = 0;	// return value usable for life of parameter

	// this is similar to getParString, but will return an absolute path if it exists, with
	// slash direction consistent with O/S requirements.
	// to get the original parameter value, use getParString
	virtual const char*	getParFilePath(const char* name) = 0;	// return value usable for life of parameter

	// returns true on success
	// from_name and to_name must be Object parameters
	virtual bool	getRelativeTransform(const char* from_name, const char* to_name, double matrix[4][4]) = 0;


	// disable or enable updating of the parameter
	virtual void		 enablePar(const char* name, bool onoff) = 0;


	// these are defined by paths.
	// may return nullptr when invalid input
	// this value is valid until the parameters are rebuilt or it is called with the same parameter name.
	virtual const OP_DATInput*		getDAT(const char *path) = 0;
	virtual const OP_TOPInput*		getTOP(const char *path) = 0;
	virtual const OP_CHOPInput*		getCHOP(const char *path) = 0;
	virtual const OP_ObjectInput*	getObject(const char *path) = 0;

};

class OP_InfoCHOPChan
{
public:
	char*			name;
	float			value;
private:
	int				reserved[10];
};

class OP_InfoDATSize
{
public:
	
	// Set this to the size you want the table to be

	int				rows;
	int				cols;

	// Set this to true if you want to return DAT entries on a column
	// by column basis.
	// Otherwise set to false, and you'll be expected to set them on
	// a row by row basis.
	// DEFAULT : false

	bool			byColumn;


private:
	int				reserved[10];
};

class OP_InfoDATEntries
{
public:

	// This is an array of char* pointers which you are expected to assign
	// The start off as NULL, you need to allocate or assign constant/statis
	// strings to them
	// e.g values[1] = "myColumnName";

	char**			values;

private:
	int				reserved[10];
};



class OP_NumericParameter
{
public:

	OP_NumericParameter(const char* iname=nullptr)
	{
		name = iname;
		label = page = nullptr;

		for (int i=0; i<4; i++)
		{
			defaultValues[i] = 0.0;

			minSliders[i] = 0.0;
			maxSliders[i] = 1.0;

			minValues[i] = 0.0;
			maxValues[i] = 1.0;

			clampMins[i] = false;
			clampMaxes[i] = false;
		}
	}

	// Any char* values passed are copied immediately by the append parameter functions,
	// and do not need to be retained by the calling function.
	
	const char*	name;		// Must begin with capital letter, and contain no spaces
	const char*	label;
	const char*	page;

	double		defaultValues[4];
	double		minValues[4];
	double		maxValues[4];

	bool		clampMins[4];
	bool		clampMaxes[4];

	double		minSliders[4];
	double		maxSliders[4];

private:

	int			reserved[20];

};

class OP_StringParameter
{
public:

	OP_StringParameter(const char* iname=nullptr)
	{
		name = iname;
		label = page = nullptr;
		defaultValue = nullptr;
	}

	// Any char* values passed are copied immediately by the append parameter functions,
	// and do not need to be retained by the calling function.

	const char*	name;		// Must begin with capital letter, and contain no spaces
	const char*	label;
	const char*	page;

	const char*	defaultValue;

private:

	int			reserved[20];
};


enum ParAppendResult
{
	PARAMETER_APPEND_SUCCESS = 0,
	PARAMETER_APPEND_INVALID_NAME = 1,	// invalid or duplicate name
	PARAMETER_APPEND_INVALID_SIZE = 2,	// size out of range
};


class OP_ParameterManager
{

public:

	// Returns PARAMETER_APPEND_SUCCESS on succesful

	virtual ParAppendResult		appendFloat(const OP_NumericParameter &np, int size=1) = 0;
	virtual ParAppendResult		appendInt(const OP_NumericParameter &np, int size=1) = 0;

	virtual ParAppendResult		appendXY(const OP_NumericParameter &np) = 0;
	virtual ParAppendResult		appendXYZ(const OP_NumericParameter &np) = 0;

	virtual ParAppendResult		appendUV(const OP_NumericParameter &np) = 0;
	virtual ParAppendResult		appendUVW(const OP_NumericParameter &np) = 0;

	virtual ParAppendResult		appendRGB(const OP_NumericParameter &np) = 0;
	virtual ParAppendResult		appendRGBA(const OP_NumericParameter &np) = 0;

	virtual ParAppendResult		appendToggle(const OP_NumericParameter &np) = 0;
	virtual ParAppendResult		appendPulse(const OP_NumericParameter &np) = 0;

	virtual ParAppendResult		appendString(const OP_StringParameter &sp) = 0;
	virtual ParAppendResult		appendFile(const OP_StringParameter &sp) = 0;
	virtual ParAppendResult		appendFolder(const OP_StringParameter &sp) = 0;

	virtual ParAppendResult		appendDAT(const OP_StringParameter &sp) = 0;
	virtual ParAppendResult		appendCHOP(const OP_StringParameter &sp) = 0;
	virtual ParAppendResult		appendTOP(const OP_StringParameter &sp) = 0;
	virtual ParAppendResult		appendObject(const OP_StringParameter &sp) = 0;

	// Any char* values passed are copied immediately by the append parameter functions,
	// and do not need to be retained by the calling function.
	virtual ParAppendResult		appendMenu(const OP_StringParameter &sp,
									int nitems, const char **names,
									const char **labels) = 0;

	// Any char* values passed are copied immediately by the append parameter functions,
	// and do not need to be retained by the calling function.
	virtual ParAppendResult		appendStringMenu(const OP_StringParameter &sp,
									int nitems, const char **names,
									const char **labels) = 0;

private:


};

#endif
