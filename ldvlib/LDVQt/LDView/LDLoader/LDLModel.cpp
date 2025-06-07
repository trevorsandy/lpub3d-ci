#include "LDLModel.h"
#include "LDLMainModel.h"
#include "LDLCommentLine.h"
#include "LDLModelLine.h"
#include "LDLFindFileAlert.h"
#include "LDrawIni.h"
#include <TCFoundation/TCDictionary.h>
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCStringArray.h>
#include <TCFoundation/TCAlertManager.h>
#include <TCFoundation/TCProgressAlert.h>
#include <TCFoundation/TCLocalStrings.h>
#include <TCFoundation/TCUserDefaults.h>
#include <TCFoundation/TCImage.h>
#include <TCFoundation/TCUnzipStream.h>
#include <TCFoundation/TCWebClient.h>
#include <math.h>

#ifdef WIN32
#include <direct.h>

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG

#else // WIN32
#include <unistd.h>
#endif // WIN32

#define LDL_LOWRES_PREFIX "LDL-LOWRES:"
#define LOAD_MESSAGE TCLocalStrings::get(_UC("LDLModelLoading"))
#define MAIN_READ_FRACTION 0.1f

typedef std::pair<std::string, LDrawSearchDirS*> SearchDirPair;
typedef std::vector<SearchDirPair> SearchDirVector;

// LPub3D Mod - stud style
#define TC_ARRAY_COUNT(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define TC_MAX_DATA_LEN 1024

struct TCStudStylePrimitive
{
	char Name[64];
	char Data[TC_MAX_DATA_LEN];
	char DataLogo1[TC_MAX_DATA_LEN];
};

static struct TCStudStylePrimitive StudStylePrimitives[] =
{
	{   /* 00 */
		"stud.dat",
		"0 Stud\n0 Name: stud.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2012-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 4242 0 0 0 6 0 0 0 -4 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4disc.dat\n",
		"0 Stud\n0 Name: stud.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2012-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 1 0 0 0 1 0 0 0 1 stud-logo.dat\n"
	},
	{   /* 01 */
		"8/stud.dat",
		"0 Stud (Fast-Draw)\n0 Name: 8\\stud.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 4242 0 0 0 6 0 0 0 -4 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4disc.dat\n",
		"0 Stud (Fast-Draw)\n0 Name: 8\\stud.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 4242 0 0 0 6 0 0 0 -4 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4disc.dat\n"
	},
	{   /* 02 */
		"stud2.dat",
		"0 Stud Open\n0 Name: stud2.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2009-02\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 4 0 0 0 1 0 0 0 4 4-4edge.dat\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 4-4cyli.dat\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 4-4ring2.dat\n0\n",
		"0 Stud Open\n0 Name: stud2.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2009-02\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 1 0 0 0 1 0 0 0 1 stud2-logo.dat\n"
	},
	{   /* 03 */
		"8/stud2.dat",
		"0 Stud Open (Fast-Draw)\n0 Name: 8\\stud2.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 4 0 0 0 1 0 0 0 4 8\4-4edge.dat\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 8\4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 8\4-4cyli.dat\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 8\4-4ring2.dat\n",
		"0 Stud Open (Fast-Draw)\n0 Name: 8\\stud2.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 4 0 0 0 1 0 0 0 4 8\4-4edge.dat\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 8\4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 8\4-4cyli.dat\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 8\4-4ring2.dat\n"
	},
	{   /* 04 */
		"stud2a.dat",
		"0 Stud Open without Base Edges\n0 Name: stud2a.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2009-02\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 4-4cyli.dat\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 4-4ring2.dat\n0\n",
		"0 Stud Open without Base Edges\n0 Name: stud2a.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2009-02\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 4-4cyli.dat\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 4-4ring2.dat\n0\n"
	},
	{   /* 05 */
		"8/stud2a.dat",
		"0 Stud Open without Base Edges (Fast-Draw)\n0 Name: 8\\stud2a.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 8\4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 8\4-4cyli.dat\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 8\4-4ring2.dat\n",
		"0 Stud Open without Base Edges (Fast-Draw)\n0 Name: 8\\stud2a.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 8\4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 8\4-4cyli.dat\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 8\4-4ring2.dat\n"
	},
	{   /* 06 */
		"stud3.dat",
		"0 Stud Tube Solid\n0 Name: stud3.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2012-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 4-4edge.dat\n1 16 0 0 0 4 0 0 0 1 0 0 0 4 4-4edge.dat\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 4-4disc.dat\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 4-4cyli.dat\n",
		"0 Stud Tube Solid\n0 Name: stud3.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2012-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 4-4edge.dat\n1 16 0 0 0 4 0 0 0 1 0 0 0 4 4-4edge.dat\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 4-4disc.dat\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 4-4cyli.dat\n"
	},
	{   /* 07 */
		"8/stud3.dat",
		"0 Stud Tube Solid (Fast-Draw)\n0 Name: 8\\stud3.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 8\4-4edge.dat\n1 16 0 0 0 4 0 0 0 1 0 0 0 4 8\4-4edge.dat\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 8\4-4disc.dat\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 8\4-4cyli.dat\n",
		"0 Stud Tube Solid (Fast-Draw)\n0 Name: 8\\stud3.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 8\4-4edge.dat\n1 16 0 0 0 4 0 0 0 1 0 0 0 4 8\4-4edge.dat\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 8\4-4disc.dat\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 8\4-4cyli.dat\n"
	},
	{   /* 08 */
		"stud4.dat",
		"0 Stud Tube Open\n0 Name: stud4.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2009-02\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 4-4edge.dat\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 0 0 8 0 0 0 1 0 0 0 8 4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 4-4ring3.dat\n0\n",
		"0 Stud Tube Open\n0 Name: stud4.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2009-02\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 4-4edge.dat\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 0 0 8 0 0 0 1 0 0 0 8 4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 4-4ring3.dat\n0\n"
	},
	{   /* 09 */
		"8/stud4.dat",
		"0 Stud Tube Open (Fast-Draw)\n0 Name: 8\\stud4.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 8\4-4edge.dat\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 0 0 8 0 0 0 1 0 0 0 8 8\4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 8\4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 8\4-4ring3.dat\n",
		"0 Stud Tube Open (Fast-Draw)\n0 Name: 8\\stud4.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 8\4-4edge.dat\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 0 0 8 0 0 0 1 0 0 0 8 8\4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 8\4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 8\4-4ring3.dat\n"
	},
	{   /* 10 */
		"stud4a.dat",
		"0 Stud Tube Open without Base Edges\n0 Name: stud4a.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2009-02\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 4-4ring3.dat\n0\n",
		"0 Stud Tube Open without Base Edges\n0 Name: stud4a.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive UPDATE 2009-02\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 4-4ring3.dat\n0\n"
	},
	{   /* 11 */
		"8/stud4a.dat",
		"0 Stud Tube Open without Base Edges (Fast-Draw)\n0 Name: 8\\stud4a.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 8\4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 8\4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 8\4-4ring3.dat\n",
		"0 Stud Tube Open without Base Edges (Fast-Draw)\n0 Name: 8\\stud4a.dat\n0 Author: James Jessiman\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 8\4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 8\4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 8\4-4ring3.dat\n"
	},
	{   /* 12 */
		"stud4h.dat",
		"0 Stud Tube Open with Extended Hole\n0 Name: stud4h.dat\n0 Author: Tim Gould [timgould]\n0 !LDRAW_ORG Primitive UPDATE 2012-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 4-4edge.dat\n1 16 0 4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 0 0 8 0 0 0 1 0 0 0 8 4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 8 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 4-4ring3.dat\n0\n",
		"0 Stud Tube Open with Extended Hole\n0 Name: stud4h.dat\n0 Author: Tim Gould [timgould]\n0 !LDRAW_ORG Primitive UPDATE 2012-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 4-4edge.dat\n1 16 0 4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 0 0 8 0 0 0 1 0 0 0 8 4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 8 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 4-4ring3.dat\n0\n"
	},
	{   /* 13 */
		"8/stud4h.dat",
		"0 Stud Tube Open with Extended Hole (Fast-Draw)\n0 Name: 8\\stud4h.dat\n0 Author: Tim Gould [timgould]\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 8\4-4edge.dat\n1 16 0 4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 0 0 8 0 0 0 1 0 0 0 8 8\4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 8 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 8\4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 8\4-4ring3.dat\n",
		"0 Stud Tube Open with Extended Hole (Fast-Draw)\n0 Name: 8\\stud4h.dat\n0 Author: Tim Gould [timgould]\n0 !LDRAW_ORG 8_Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 8\4-4edge.dat\n1 16 0 4 0 6 0 0 0 1 0 0 0 6 8\4-4edge.dat\n1 16 0 0 0 8 0 0 0 1 0 0 0 8 8\4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 8 0 0 0 6 8\4-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 8\4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 8\4-4ring3.dat\n"
	},
	{   /* 14 */
		"2-4stud4.dat",
		"0 Stud Tube Open 0.5\n0 Name: 2-4stud4.dat\n0 Author: Joerg Sommerer [Brickaneer]\n0 !LDRAW_ORG Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 2-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 2-4edge.dat\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 2-4edge.dat\n1 16 0 0 0 8 0 0 0 1 0 0 0 8 2-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 2-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 2-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 2-4ring3.dat\n",
		"0 Stud Tube Open 0.5\n0 Name: 2-4stud4.dat\n0 Author: Joerg Sommerer [Brickaneer]\n0 !LDRAW_ORG Primitive UPDATE 2016-01\n\n0 BFC CERTIFY CCW\n\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 2-4edge.dat\n1 16 0 -4 0 8 0 0 0 1 0 0 0 8 2-4edge.dat\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 2-4edge.dat\n1 16 0 0 0 8 0 0 0 1 0 0 0 8 2-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 2-4cyli.dat\n1 16 0 -4 0 8 0 0 0 4 0 0 0 8 2-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 2-4ring3.dat\n"
	},
	{   /* 15 */
		"stud-logo.dat",
		"",
		"0 Stud with LEGO Logo - Non-3D Thin Lines\n0 Name: stud-logo.dat\n0 Author: Paul Easter [pneaster]\n0 !LDRAW_ORG Unofficial_Primitive\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 4242 0 0 0 6 0 0 0 -4 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4disc.dat\n\n1 16 0 -4 0 1 0 0 0 1 0 0 0 1 logo.dat\n"
	},
	{   /* 16 */
		"8/stud-logo.dat",
		"",
		"0 Stud with LEGO Logo - Non-3D Thin Lines (Fast-Draw)\n0 Name: 8\\stud-logo.dat\n0 Author: Steffen [Steffen]\n0 !LDRAW_ORG Unofficial_8_Primitive\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 1 0 0 0 1 0 0 0 1 8\\stud.dat\n"
	},
	{   /* 17 */
		"stud2-logo.dat",
		"",
		"0 Stud Open with LEGO Logo - Non-3D Thin Lines\n0 Name: stud2-logo.dat\n0 Author: Paul Easter [pneaster]\n0 !LDRAW_ORG Unofficial_Primitive\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 4 0 0 0 1 0 0 0 4 4-4edge.dat\n1 16 0 0 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n1 16 0 -4 0 4 0 0 0 1 0 0 0 4 4-4edge.dat\n1 16 0 -4 0 6 0 0 0 1 0 0 0 6 4-4edge.dat\n0 BFC INVERTNEXT\n1 4242 0 -4 0 4 0 0 0 4 0 0 0 4 4-4cyli.dat\n1 4242 0 -4 0 6 0 0 0 4 0 0 0 6 4-4cyli.dat\n1 16 0 -4 0 2 0 0 0 1 0 0 0 2 4-4ring2.dat\n\n1 16 0 0 0 0.6 0 0 0 1 0 0 0 0.6 logo.dat\n"
	},
	{   /* 18 */
		"8/stud2-logo.dat",
		"",
		"0 Stud Open with LEGO Logo - Non-3D Thin Lines (Fast-Draw)\n0 Name: 8\\stud2-logo.dat\n0 Author: Steffen [Steffen]\n0 !LDRAW_ORG Unofficial_8_Primitive\n\n0 BFC CERTIFY CCW\n\n1 16 0 0 0 1 0 0 0 1 0 0 0 1 8\\stud2.dat\n"
	}
};
// LPub3D Mod End

char *LDLModel::sm_systemLDrawDir = NULL;
#ifdef WIN32
std::string LDLModel::sm_systemLDrawDirSlashes;
#endif // WIN32
char *LDLModel::sm_defaultLDrawDir = NULL;
LDrawIniS *LDLModel::sm_lDrawIni = NULL;
int LDLModel::sm_modelCount = 0;
bool LDLModel::sm_studCylinderColorEnabled = true;
LDLFileCaseCallback LDLModel::fileCaseCallback = NULL;
LDLModel::LDLModelCleanup LDLModel::sm_cleanup;
StringList LDLModel::sm_checkDirs;
std::string LDLModel::sm_ldrawZipPath;
std::string LDLModel::sm_unoffZipPath;
bool LDLModel::sm_verifyLDrawSubDirs = false;
#ifdef HAVE_MINIZIP
unzFile LDLModel::sm_ldrawZip = NULL;
unzFile LDLModel::sm_unoffZip = NULL;
#endif // HAVE_MINIZIP

LDLModel::LDLModelCleanup::~LDLModelCleanup(void)
{
	delete[] LDLModel::sm_systemLDrawDir;
	delete[] LDLModel::sm_defaultLDrawDir;
	LDLModel::sm_systemLDrawDir = NULL;
	if (LDLModel::sm_lDrawIni)
	{
		LDrawIniFree(LDLModel::sm_lDrawIni);
	}
	closeZips();
}


LDLModel::LDLModel(void)
	:m_filename(NULL),
	m_name(NULL),
	m_author(NULL),
	m_description(NULL),
	m_fileLines(NULL),
	m_mpdTexmapModels(NULL),
	m_mpdTexmapLines(NULL),
	m_mpdTexmapImages(NULL),
	m_mainModel(NULL),
	m_activeLineCount(0),
	m_activeMPDModel(NULL),
	m_maxRadius(0.0f),
	m_maxFullRadius(0.0f),
	m_texmapImage(NULL),
	m_dataLine(NULL)
{
	memset(&m_flags, 0, sizeof(m_flags));
	// Initialize Private flags
	m_flags.loadingPart = false;
	m_flags.loadingSubPart = false;
	m_flags.loadingPrimitive = false;
	m_flags.loadingUnoffic = false;
	m_flags.loadingFoundFile = false;
	m_flags.mainModelLoaded = false;
	m_flags.mainModelParsed = false;
	m_flags.started = false;
	m_flags.bfcClip = false;
	m_flags.bfcWindingCCW = true;
	m_flags.bfcInvertNext = false;
	m_flags.haveBoundingBox = false;
	m_flags.haveMaxRadius = false;
	m_flags.haveMaxFullRadius = false;
	m_flags.fullRadius = false;
	m_flags.texmapStarted = false;
	m_flags.texmapFallback = false;
	m_flags.texmapNext = false;
	// Initialize Public flags
	m_flags.part = false;
	m_flags.subPart = false;
	m_flags.primitive = false;
	m_flags.mpd = false;
	m_flags.noShrink = false;
	m_flags.official = false;
	m_flags.unofficial = false;
	m_flags.hasStuds = false;
	m_flags.bfcCertify = BFCUnknownState;
	m_flags.bboxIgnoreOn = false;
	m_flags.bboxIgnoreBegun = false;
	sm_modelCount++;
	memset(m_texmapExtra, 0, sizeof(m_texmapExtra));
}

LDLModel::LDLModel(const LDLModel &other)
	:m_filename(copyString(other.m_filename)),
	m_name(copyString(other.m_name)),
	m_author(copyString(other.m_author)),
	m_description(copyString(other.m_description)),
	m_fileLines(NULL),
	m_mpdTexmapModels(NULL),
	m_mpdTexmapLines(NULL),
	m_mpdTexmapImages(NULL),
	m_mainModel(other.m_mainModel),
	m_stepIndices(other.m_stepIndices),
	m_activeLineCount(other.m_activeLineCount),
	m_activeMPDModel(NULL),
	m_boundingMin(other.m_boundingMin),
	m_boundingMax(other.m_boundingMax),
	m_center(other.m_center),
	m_maxRadius(other.m_maxRadius),
	m_maxFullRadius(other.m_maxFullRadius),
	m_texmapImage(TCObject::retain(other.m_texmapImage)),
	m_dataLine(TCObject::retain(other.m_dataLine)),
	m_flags(other.m_flags)
{
	memcpy(m_texmapExtra, other.m_texmapExtra, sizeof(m_texmapExtra));
	for (size_t i = 0; i < 3; ++i)
	{
		m_texmapPoints[i] = other.m_texmapPoints[i];
	}
	if (other.m_fileLines)
	{
		m_fileLines = (LDLFileLineArray *)other.m_fileLines->copy();
	}
	if (other.m_mpdTexmapModels)
	{
		m_mpdTexmapModels = (LDLModelArray *)other.m_mpdTexmapModels->copy();
	}
	if (other.m_mpdTexmapLines)
	{
		m_mpdTexmapLines = (LDLCommentLineArray *)other.m_mpdTexmapLines->copy();
	}
	if (other.m_mpdTexmapImages)
	{
		m_mpdTexmapImages = (TCImageArray *)other.m_mpdTexmapImages->copy();
	}
}

void LDLModel::dealloc(void)
{
	delete[] m_filename;
	delete[] m_name;
	delete[] m_author;
	delete[] m_description;
	TCObject::release(m_fileLines);
	TCObject::release(m_mpdTexmapModels);
	TCObject::release(m_mpdTexmapLines);
	TCObject::release(m_mpdTexmapImages);
	TCObject::release(m_texmapImage);
	TCObject::release(m_dataLine);
	sm_modelCount--;
	TCObject::dealloc();
}

TCObject *LDLModel::copy(void) const
{
	return new LDLModel(*this);
}

void LDLModel::setFilename(const char *filename)
{
	delete[] m_filename;
	m_filename = copyString(filename);
}

void LDLModel::setName(const char *name)
{
	delete[] m_name;
	m_name = copyString(name);
}

TCULong LDLModel::getPackedRGBA(int colorNumber)
{
	int r, g, b, a;

	getRGBA(colorNumber, r, g, b, a);
	return r << 24 | g << 16 | b << 8 | a;
}

bool LDLModel::hasSpecular(int colorNumber)
{
	return m_mainModel->hasSpecular(colorNumber);
}

bool LDLModel::hasShininess(int colorNumber)
{
	return m_mainModel->hasShininess(colorNumber);
}

void LDLModel::getSpecular(int colorNumber, float *specular)
{
	m_mainModel->getSpecular(colorNumber, specular);
}

void LDLModel::getShininess(int colorNumber, float &shininess)
{
	m_mainModel->getShininess(colorNumber, shininess);
}

int LDLModel::getEdgeColorNumber(int colorNumber)
{
	return m_mainModel->getEdgeColorNumber(colorNumber);
}

void LDLModel::getRGBA(int colorNumber, int& r, int& g, int& b, int& a)
{
	m_mainModel->getRGBA(colorNumber, r, g, b, a);
}

bool LDLModel::colorNumberIsTransparent(int colorNumber)
{
	return m_mainModel->colorNumberIsTransparent(colorNumber);
}

LDLModel *LDLModel::subModelNamed(const char *subModelName, bool lowRes,
								  bool secondAttempt,
								  const LDLModelLine *fileLine, bool knownPart)
{
	TCDictionary* subModelDict = getLoadedModels();
	LDLModel* subModel;
	char *dictName = NULL;
	char *adjustedName;
	bool &ancestorCheck = m_mainModel->ancestorCheck(subModelName);
	bool loop = false;

	if (ancestorCheck)
	{
		// Recursion: the model named subModelName is an ancestor of the current
		// model.  Loading it will result in an infinite loop and crash after
		// the stack overflows.
		return NULL;
	}
	else
	{
		ancestorCheck = true;
	}
	// LPub3D Mod - stud style
	if (isStudStylePrimitive(subModelName, TCUserDefaults::longForKey("StudStyle", 0)))
	{
		// LPub3D Mod End
		m_flags.hasStuds = true;
	}
	adjustedName = copyString(subModelName);
	replaceStringCharacter(adjustedName, '\\', '/');
	if (lowRes)
	{
		if (stringHasCaseInsensitivePrefix(adjustedName, "stud"))
		{
			adjustedName[3] = '2';
		}
		else
		{
			delete[] adjustedName;
			ancestorCheck = false;
			return NULL;
		}
		size_t dictNameLen = strlen(LDL_LOWRES_PREFIX) + strlen(adjustedName) + 1;
		dictName = new char[dictNameLen];
		snprintf(dictName, dictNameLen, "%s%s", LDL_LOWRES_PREFIX, adjustedName);
	}
	else
	{
		dictName = copyString(adjustedName);
	}
	subModel = (LDLModel*)(subModelDict->objectForKey(dictName));
	if (subModel == NULL)
	{
		std::ifstream subModelStream;
		TCUnzipStream zipStream;
		std::string subModelPath;

		if (openSubModelNamed(adjustedName, subModelPath, subModelStream,
			&zipStream, knownPart, &loop))
		{
			bool clearSubModel = false;
			replaceStringCharacter(&subModelPath[0], '\\', '/');
			subModel = new LDLModel;
			subModel->setFilename(subModelPath.c_str());

			if (!initializeNewSubModel(subModel, dictName, subModelStream,
				&zipStream))
			{
				clearSubModel = true;
			}
			if (clearSubModel)
			{
				subModel = NULL;
			}
			m_flags.loadingPart = false;
			m_flags.loadingSubPart = false;
			m_flags.loadingUnoffic = false;
		}
	}
	if (subModel != NULL && subModel->isUnOfficial())
	{
		sendUnofficialWarningIfPart(subModel, fileLine, subModelName);
	}
	delete[] adjustedName;
	if (!subModel && !secondAttempt && !loop)
	{
		LDLFindFileAlert *alert = new LDLFindFileAlert(subModelName);

		TCAlertManager::sendAlert(alert, this);
		if (alert->getFileFound())
		{
			m_flags.loadingFoundFile = true;
			subModel = subModelNamed(alert->getFilename().c_str(), lowRes,
				true, fileLine, alert->getPartFlag());
			m_flags.loadingFoundFile = false;
			if (subModel)
			{
				// The following is necessary in order for primitive
				// substitution to work.
				subModel->setName(dictName);
				sendUnofficialWarningIfPart(subModel, fileLine, subModelName);
			}
		}
		else if (alert->getTooManyRequestsFlag())
		{
			reportError(LDLETooManyRequests, *fileLine,
				TCLocalStrings::get(_UC("LDLModelTooManyRequests")));
		}
		alert->release();
	}
	delete[] dictName;
	if (!loop)
	{
		ancestorCheck = false;
	}
	return subModel;
}

void LDLModel::sendUnofficialWarningIfPart(
	const LDLModel *subModel,
	const LDLModelLine *fileLine,
	const char *subModelName)
{
	if (!isPart() && subModel->isPart())
	{
		UCCHAR szWarning[1024];
		UCSTR ucSubModelName = mbstoucstring(subModelName);

		sucprintf(szWarning, COUNT_OF(szWarning),
			TCLocalStrings::get(_UC("LDLModelUnofficialPart")),
			ucSubModelName);
		delete[] ucSubModelName;
		reportWarning(LDLEUnofficialPart, *fileLine, szWarning);
	}
}

// NOTE: static function
bool LDLModel::openStream(const char *filename, std::ifstream &stream)
{
	// Use binary mode to work with DOS and Unix line endings and allow
	// seeking in the file.  The file parsing code will still work fine and
	// strip out the extra data.
#ifdef _MSC_VER
	ucstring ucFilename;
	utf8toucstring(ucFilename, filename);
	if (!ucFilename.empty())
	{
		// Windows STL has a non-standard extension to support wide-character
		// filenames
		stream.open(ucFilename.c_str(), std::ios_base::binary);
	}
	else
	{
		// If the filename isn't valid UTF-8, try to open it using
		// the original non-wide version.
		stream.open(filename, std::ios_base::binary);
	}
#else // _MSC_VER
	stream.open(filename, std::ios_base::binary);
#endif // !_MSC_VER
	return stream.is_open() && !stream.fail();
}

bool LDLModel::isInLDrawDir(const std::string& filename)
{
#ifdef WIN32
	return stringHasCaseInsensitivePrefix(filename.c_str(), sm_systemLDrawDir) ||
		stringHasCaseInsensitivePrefix(filename.c_str(), sm_systemLDrawDirSlashes.c_str());
#else
	return stringHasCaseInsensitivePrefix(filename.c_str(), sm_systemLDrawDir);
#endif // WIN32
}

// NOTE: static function
bool LDLModel::fileExists(const std::string &filename)
{
#ifdef HAVE_MINIZIP
	if (sm_ldrawZip != NULL &&
		sm_systemLDrawDir != NULL &&
		isInLDrawDir(filename))
	{
		std::string lfilename = lowerCaseString(filename);
		TCUnzipStream zipStream;
		std::string partPath = &lfilename[strlen(sm_systemLDrawDir) + 1];
		if (stringHasPrefix(partPath, "unofficial/"))
		{
			if (sm_unoffZip != NULL)
			{
				partPath = partPath.substr(11);
				time_t localTimestamp = getLocalTimestamp(partPath);
				if (zipStream.getTimestamp(sm_unoffZipPath, sm_unoffZip, partPath) >= localTimestamp)
				{
					return true;
				}
			}
		}
		else
		{
			if (zipStream.getTimestamp(sm_ldrawZipPath, sm_ldrawZip, partPath) != 0)
			{
				return true;
			}
		}
	}
#endif // HAVE_MINIZIP
	FILE* file = ucfopen(filename.c_str(), "r");

	if (file)
	{
		fclose(file);
		return true;
	}
	else
	{
		return false;
	}
}

// NOTE: static function
std::string LDLModel::getLastModifiedKey(const std::string& lfilename)
{
	size_t slashSpot = lfilename.find('/');
	std::string partName = lfilename;
	if (slashSpot != lfilename.npos)
	{
		partName = lfilename.substr(slashSpot + 1);
	}
	return "UnofficialPartChecks/" + partName + "/LastModified";
}

time_t LDLModel::getFileTimestamp(const std::string& path)
{
	time_t result = 0;
	TCStat statBuf;
	if (ucstat(path.c_str(), &statBuf) == 0)
	{
#if (!defined(_POSIX_C_SOURCE) && !defined(_POSIX_SOURCE) && !defined(WIN32)) || defined(_DARWIN_C_SOURCE)
		result = statBuf.st_mtimespec.tv_sec;
#else
		result = statBuf.st_mtime;
#endif
	}
	return result;
}

// NOTE: static function
time_t LDLModel::getLocalTimestamp(const std::string& lfilename)
{
	std::string lastModifiedKey = getLastModifiedKey(lfilename);
	char *localLastModified = TCUserDefaults::stringForKey(lastModifiedKey.c_str(), NULL, false);
	time_t localTimestamp = 0;
	if (localLastModified != NULL)
	{
		localTimestamp = TCWebClient::scanDateString(localLastModified);
		delete[] localLastModified;
	}
	if (localTimestamp == 0)
	{
		std::string base = unofficialDir();
		if (base.empty())
		{
			return localTimestamp;
		}
		std::string path;
		combinePath(base, lfilename, path);
		localTimestamp = getFileTimestamp(path);
		if (localTimestamp == 0 && fileCaseCallback != NULL && fileCaseCallback(&path[0]))
		{
			localTimestamp = getFileTimestamp(path);
		}
	}
	return localTimestamp;
}

// NOTE: static function
bool LDLModel::openFile(
	std::string &filename,
	std::ifstream &modelStream,
	TCUnzipStream *zipStream,
	bool canLoadUnofficial)
{
	std::string lfilename = lowerCaseString(filename);
#ifdef HAVE_MINIZIP
	if (sm_ldrawZip != NULL &&
		sm_systemLDrawDir != NULL &&
		zipStream != NULL &&
		isInLDrawDir(filename))
	{
		std::string partPath = &lfilename[strlen(sm_systemLDrawDir) + 1];
		if (stringHasPrefix(partPath, "unofficial/"))
		{
			if (sm_unoffZip != NULL)
			{
				partPath = partPath.substr(11);
				time_t zipTimestamp = zipStream->getTimestamp(sm_unoffZipPath, sm_unoffZip, partPath);
				if (zipTimestamp != 0)
				{
					time_t localTimestamp = getLocalTimestamp(partPath);
					std::string origPartPath = &filename[strlen(sm_systemLDrawDir) + 1];
					// The main part loading calls this function with
					// canLoadUnofficial set to false if unofficial downloads
					// are enabled. We then look in ldrawunf.zip to see if it
					// contains the file. If it does, and we don't already have
					// a downloaded file in the Unofficial directory that is
					// newer, we store the timestamp from the zip in
					// TCUserDefaults and return.
					// Later on, the Unofficial download code calls this with
					// canLoadUnofficial set to true. At that point we'll load
					// the file from the zip if it is at least as new as the
					// stored timestamp for the file, or load the file from the
					// disk otherwise.
					if (!canLoadUnofficial)
					{
						if (zipTimestamp > localTimestamp)
						{
							char* rfc822 = TCWebClient::toRfc822(zipTimestamp);
							if (rfc822 != NULL)
							{
								std::string lastModifiedKey = getLastModifiedKey(partPath);
								TCUserDefaults::setStringForKey(rfc822, lastModifiedKey.c_str(), false);
								delete[] rfc822;
							}
						}
						return false;
					}
					else if (zipTimestamp >= localTimestamp)
					{
						if (zipStream->load(sm_unoffZipPath, sm_unoffZip, partPath))
						{
							filename = sm_unoffZipPath + ":" + partPath;
							return true;
						}
					}
				}
			}
		}
		else
		{
			std::string zipPath = std::string("ldraw/") + partPath;
			if (zipStream->load(sm_ldrawZipPath, sm_ldrawZip, zipPath))
			{
				filename = sm_ldrawZipPath + ":" + zipPath;
				return true;
			}
			if (stringHasPrefix(partPath, "p/") ||
				stringHasPrefix(partPath, "parts/"))
			{
				return false;
			}
		}
	}
#endif // HAVE_MINIZIP
	if (fileCaseCallback)
	{
		if (openStream(lfilename.c_str(), modelStream))
		{
			return true;
		}
		convertStringToUpper(lfilename);
		if (openStream(lfilename.c_str(), modelStream))
		{
			return true;
		}
		lfilename = filename;
		if (openStream(lfilename.c_str(), modelStream))
		{
			return true;
		}
		if (fileCaseCallback(&lfilename[0]))
		{
			openStream(lfilename.c_str(), modelStream);
		}
	}
	else
	{
		openStream(lfilename.c_str(), modelStream);
	}
	return modelStream.is_open();
}

bool LDLModel::openModelFile(
	std::string &filename,
	std::ifstream &modelStream,
	TCUnzipStream *zipStream,
	bool isText,
	bool knownPart /*= false*/)
{
	if (openFile(filename, modelStream, zipStream, m_flags.loadingFoundFile || !m_mainModel->getCheckPartTracker()))
	{
		if (knownPart)
		{
			m_flags.loadingPart = true;
		}
		if (isText)
		{
			// Check for UTF-8 Byte order mark (BOM), and skip over it if
			// present. Only do this on text files. (Right now, texture maps
			// are the only binary files that get opened by this function.)
			if (zipStream != NULL && zipStream->is_valid())
			{
				skipUtf8BomIfPresent(*zipStream);
			}
			else
			{
				skipUtf8BomIfPresent(modelStream);
			}
		}
	}
	return (zipStream != NULL && zipStream->is_valid()) ||
		(modelStream.is_open() && !modelStream.fail());
}

bool LDLModel::isSubPart(const char *subModelName)
{
	return stringHasCaseInsensitivePrefix(subModelName, "s/") ||
		stringHasCaseInsensitivePrefix(subModelName, "s\\");
}

bool LDLModel::isAbsolutePath(const char *path)
{
#ifdef WIN32
	return strlen(path) >= 2 && path[1] == ':' || path[0] == '/';
#else
	return path[0] == '/';
#endif
}

// NOTE: static function.
void LDLModel::combinePathParts(
	std::string &path,
	const std::string &left,
	const std::string& middle,
	const std::string &right)
{
	path = left;
	path += middle;
	path += right;
}

bool LDLModel::openSubModelNamed(
	const char* subModelName,
	std::string &subModelPath,
	std::ifstream &subModelStream,
	TCUnzipStream *zipStream,
	bool knownPart,
	bool *pLoop /*= NULL*/,
	bool isText /*= true*/)
{
	TCStringArray *extraSearchDirs = m_mainModel->getExtraSearchDirs();

	if (pLoop != NULL)
	{
		*pLoop = false;
	}
	subModelPath = subModelName;
	if (isAbsolutePath(subModelPath.c_str()))
	{
		return openModelFile(subModelPath, subModelStream, zipStream,
			isText, knownPart);
	}
	SearchDirVector dirs;
#ifdef HAVE_MINIZIP
	if (sm_ldrawZip != NULL)
	{
		std::string root = sm_systemLDrawDir;
		dirs.push_back(std::make_pair<std::string, LDrawSearchDirS*>(root + "/p", NULL));
		dirs.push_back(std::make_pair<std::string, LDrawSearchDirS*>(root + "/parts", NULL));
		dirs.push_back(std::make_pair<std::string, LDrawSearchDirS*>(root + "/models", NULL));
		dirs.push_back(std::make_pair<std::string, LDrawSearchDirS*>(root + "/unofficial/p", NULL));
		dirs.push_back(std::make_pair<std::string, LDrawSearchDirS*>(root + "/unofficial/parts", NULL));
	}
#endif // HAVE_MINIZIP
	if (sm_lDrawIni != NULL && sm_lDrawIni->nSearchDirs > 0)
	{
		for (int i = 0; i < sm_lDrawIni->nSearchDirs; i++)
		{
			LDrawSearchDirS *searchDir = &sm_lDrawIni->SearchDirs[i];
			dirs.push_back(std::make_pair(std::string(searchDir->Dir), searchDir));
		}
	}
	for (SearchDirVector::const_iterator it = dirs.begin(); it != dirs.end(); ++it)
	{
		const std::string& dir = it->first;
		const LDrawSearchDirS *searchDir = it->second;
		bool skip = false;

		if (searchDir != NULL && searchDir->Flags & LDSDF_UNOFFIC)
		{
			if (m_mainModel->getCheckPartTracker())
			{
				skip = true;
			}
			else
			{
				m_flags.loadingUnoffic = true;
			}
		}
		if (searchDir == NULL || ((searchDir->Flags & LDSDF_SKIP) == 0 && !skip))
		{
			combinePathParts(subModelPath, dir.c_str(), "/",
				subModelName);
			if (openModelFile(subModelPath, subModelStream,
				zipStream, isText))
			{
				std::string mainModelPath(m_mainModel->getFilename());
				bool isPrimitive = false;
				bool isPart = false;
				if (searchDir != NULL)
				{
					isPrimitive = (searchDir->Flags & LDSDF_DEFPRIM) != 0;
					isPart = (searchDir->Flags & LDSDF_DEFPART) != 0;
				}
				else
				{
					isPrimitive = dir == dirs[0].first;
					isPart = dir == dirs[1].first;
				}
#ifdef WIN32
				replaceStringCharacter(mainModelPath, '\\', '/');
				replaceStringCharacter(&subModelPath[0], '\\', '/');
#endif // WIN32
				if (strcasecmp(mainModelPath.c_str(), subModelPath.c_str()) == 0)
				{
					// Recursive call to main model.
					subModelStream.close();
					if (pLoop != NULL)
					{
						*pLoop = true;
					}
					return false;
				}
				if (isPrimitive)
				{
					m_flags.loadingPrimitive = true;
				}
				else if (isPart)
				{
					if (isSubPart(subModelName))
					{
						m_flags.loadingSubPart = true;
					}
					else
					{
						m_flags.loadingPart = true;
					}
				}
				return true;
			}
		}
	}
	if (sm_lDrawIni == NULL || sm_lDrawIni->nSearchDirs == 0)
	{
		if (openModelFile(subModelPath, subModelStream, zipStream,
			isText))
		{
			return true;
		}
		combinePathParts(subModelPath, lDrawDir(), "/P/", subModelName);
		if (openModelFile(subModelPath, subModelStream, zipStream,
			isText))
		{
			m_flags.loadingPrimitive = true;
			return true;
		}
		combinePathParts(subModelPath, lDrawDir(), "/PARTS/", subModelName);
		if (openModelFile(subModelPath, subModelStream, zipStream,
			isText))
		{
			if (isSubPart(subModelName))
			{
				m_flags.loadingSubPart = true;
			}
			else
			{
				m_flags.loadingPart = true;
			}
			return true;
		}
		combinePathParts(subModelPath, lDrawDir(), "/MODELS/", subModelName);
		if (openModelFile(subModelPath, subModelStream, zipStream,
			isText))
		{
			return true;
		}
	}
	if (extraSearchDirs)
	{
		size_t i;
		size_t count = extraSearchDirs->getCount();

		for (i = 0; i < count; i++)
		{
			combinePathParts(subModelPath, (*extraSearchDirs)[i], "/",
				subModelName);
			if (openModelFile(subModelPath, subModelStream, zipStream,
				isText))
			{
				return true;
			}
		}
	}
	return false;
}

// LPub3D Mod - stud style
char *LDLModel::setCylinderColor(char *input)
{
	if (sm_studCylinderColorEnabled)
		return input;

	const char *substring = "4242";
	const char *replace = "16";
	int     number_of_matches = 0;
	size_t  substring_size = strlen(substring), replace_size = strlen(replace), buffer_size;
	char    *buffer, *bp, *ip;

	if (substring_size)
	{
		ip = strstr(input, substring);
		while (ip != NULL)
		{
			number_of_matches++;
			ip = strstr(ip+substring_size, substring);
		}
	}
	else
		number_of_matches = strlen (input) + 1;

	buffer_size = strlen(input) + number_of_matches*(replace_size - substring_size) + 1;

	if ((buffer = ((char *) malloc(buffer_size))) == NULL)
	{
		return NULL;
	}

	bp = buffer;
	ip = strstr(input, substring);
	while ((ip != NULL) && (*input != '\0'))
	{
		if (ip == input)
		{
			memcpy (bp, replace, replace_size+1);
			bp += replace_size;
			if (substring_size)
				input += substring_size;
			else
				*(bp++) = *(input++);
			ip = strstr(input, substring);
		}
		else
			while (input != ip)
				*(bp++) = *(input++);
	}

	if (substring_size)
		strcpy (bp, input);
	else
		memcpy (bp, replace, replace_size+1);

	return buffer;
}

int LDLModel::getStudStyleFile(LDLModel* subModel, const char* dictName, int studStyle, bool openStud)
{
	char data[TC_MAX_DATA_LEN];
	char tempPath[256];
	sm_studCylinderColorEnabled = TCUserDefaults::boolForKey("StudCylinderColorEnabled", true);
#ifdef WIN32
	sprintf(tempPath, "%s\\ldview_stud_style%d_%d_%s", getenv("TEMP"), studStyle, sm_studCylinderColorEnabled, dictName);
#else
	sprintf(tempPath, "/tmp/ldview_stud_style%d_%d_%s", studStyle, sm_studCylinderColorEnabled, dictName);
#endif

	std::ifstream existingSubModelStream(tempPath, std::ios::binary);
	if (existingSubModelStream.is_open())
		return (subModel->load(existingSubModelStream));

	if (studStyle < 6)
	{
		char style[10] = "";
		if (studStyle > 1)
			sprintf(style, "%d", studStyle);
		sprintf(data, "0 Stud %s\n0 Name: %s\n0 Author: James Jessiman\n0 !LDRAW_ORG Primitive\n0 BFC CERTIFY CCW\n1 16 0 0 0 1 0 0 0 1 0 0 0 1 stud%s-logo%s.dat\n", (openStud ? "open" : style), dictName, (openStud ? "2" : ""), style);
	}
	else
	{
		for (unsigned int i = 0; i < TC_ARRAY_COUNT(StudStylePrimitives); i++)
		{
			if (strcasecmp(dictName, StudStylePrimitives[i].Name) == 0)
			{
				if (studStyle == 6)
					strcpy(data, setCylinderColor(StudStylePrimitives[i].Data));
				else
					strcpy(data, setCylinderColor(StudStylePrimitives[i].DataLogo1));
				break;
			}
		}
	}

	std::ofstream outputStream;
	outputStream.open(tempPath, std::ios_base::trunc | std::ios_base::out);
	outputStream << data;
	outputStream.close();

	std::ifstream newSubModelStream(tempPath, std::ios::binary);
	return (newSubModelStream.is_open() && subModel->load(newSubModelStream));
}

int LDLModel::isStudStylePrimitive(const char* FileName, int studStyle)
{
	if (studStyle < 6)              // for styles 0-5, return if file has '-logo' suffix
		if (strstr(FileName, "-logo") != NULL)
			return 0;

	for (unsigned int i = 0; i < TC_ARRAY_COUNT(StudStylePrimitives); i++)
	{
		if (studStyle < 6 && i > 5) // for styles 0-5, return after checking 'stud', 'stud2' and 'stud2a'
			return 0;

		if (strcasecmp(FileName, StudStylePrimitives[i].Name) == 0)
		{
			if (strcasecmp(FileName, "stud2.dat") == 0 || strcasecmp(FileName, "stud2a.dat") == 0)
				return 2;           // open stud
			else
				return 1;
		}
	}
	return 0;
}
// LPub3D Mod End

bool LDLModel::initializeNewSubModel(LDLModel *subModel, const char *dictName)
{
	std::ifstream closedStream;
	return initializeNewSubModel(subModel, dictName, closedStream, NULL);
}

bool LDLModel::initializeNewSubModel(
	LDLModel *subModel,
	const char *dictName,
	std::ifstream &subModelStream,
	TCUnzipStream *zipStream)
{
	TCDictionary* subModelDict = getLoadedModels();

	subModelDict->setObjectForKey(subModel, dictName);
	subModel->release();
	subModel->m_mainModel = m_mainModel;
	if (m_flags.loadingPart)
	{
		subModel->m_flags.part = true;
		subModel->m_flags.subPart = false;
//		subModel->m_flags.bfcCertify = BFCForcedOnState;
	}
	if (m_flags.loadingSubPart)
	{
		subModel->m_flags.part = false;
		subModel->m_flags.subPart = true;
	}
	if (m_flags.loadingPrimitive)
	{
		subModel->m_flags.primitive = true;
	}
	if (m_flags.loadingUnoffic)
	{
		subModel->m_flags.unofficial = true;
	}
// LPub3D Mod - stud style
	unsigned int studStylePrimitive = 0;
	unsigned int studStyle = TCUserDefaults::longForKey("StudStyle", 0);
	if (studStyle && m_flags.loadingPrimitive && m_flags.hasStuds)
	{
		unsigned int studStyleType = isStudStylePrimitive(dictName, studStyle);
		if (studStyleType > 0)
		{
			bool openStud = studStyleType == 2;
			studStylePrimitive = getStudStyleFile(subModel, dictName, studStyle, openStud);
		}
	}
// LPub3D Mod End
	bool zipValid = zipStream != NULL && zipStream->is_valid();
// LPub3D Mod - stud style
	if (!studStylePrimitive && (subModelStream.is_open() || zipValid) &&
		!subModel->load(subModelStream, zipStream))
	{
// LPub3D Mod End
		subModelDict->removeObjectForKey(dictName);
		return false;
	}
	subModel->setName(dictName);
	return true;
}

// NOTE: static function.
bool LDLModel::verifyLDrawDir(const char *value)
{
	if (strlen(value) > 0)
	{
		char currentDir[1024];
		bool retValue = false;

		if (value && getcwd(currentDir, sizeof(currentDir)))
		{
			if (chdir(value) == 0)
			{
				if (!sm_verifyLDrawSubDirs || (chdir("parts") == 0 && chdir("..") == 0 && chdir("p") == 0))
				{
					retValue = true;
				}
			}
			if (chdir(currentDir) != 0)
			{
				debugPrintf("Error going back to original directory.\n");
				debugPrintf("currentDir before: <%s>\n", currentDir);
				if (getcwd(currentDir, sizeof(currentDir)) != NULL)
				{
					debugPrintf("currentDir  after: <%s>\n", currentDir);
				}
			}
		}
		return retValue;
	}
	else
	{
		return checkLDrawZipPath(sm_ldrawZipPath.c_str());
	}
}

// NOTE: static function.
void LDLModel::setFileCaseCallback(LDLFileCaseCallback value)
{
	fileCaseCallback = value;
	// If bool isn't 1 byte, then we can't support the case callback, so don't
	// even try.  This will trigger an error in LDLMainModel if the P and/or
	// PARTS directory inside the LDraw directory aren't capitalized.
	if (sizeof(bool) == sizeof(char))
	{
		if (sm_lDrawIni)
		{
			LDrawIniSetFileCaseCallback(fileCaseCallback);
		}
	}
}

bool LDLModel::setLDrawZipPath(const std::string& value)
{
#ifdef HAVE_MINIZIP
	sm_ldrawZipPath = value;
	ldrawZipUpdated();
	if (sm_ldrawZip == NULL && !sm_ldrawZipPath.empty())
	{
		sm_ldrawZipPath.clear();
		return false;
	}
	return true;
#else
	// Without Minizip, the only valid value is an empty string.
	return value.empty();
#endif // HAVE_MINIZIP
}

bool LDLModel::checkLDrawZipPath(const std::string& value)
{
#ifdef HAVE_MINIZIP
	if (value.empty())
	{
		return true;
	}
	unzFile testZip = TCUnzipStream::open(value.c_str());
	if (testZip != NULL)
	{
		TCUnzipStream::close(testZip);
		return true;
	}
	return false;
#else
	// Without Minizip, the only valid value is an empty string.
	return value.empty();
#endif // HAVE_MINIZIP
}

void LDLModel::closeZips(void)
{
#ifdef HAVE_MINIZIP
	if (sm_ldrawZip != NULL)
	{
		TCUnzipStream::close(sm_ldrawZip);
		sm_ldrawZip = NULL;
	}
	if (sm_unoffZip != NULL)
	{
		TCUnzipStream::close(sm_unoffZip);
		sm_unoffZip = NULL;
	}
#endif // HAVE_MINIZIP
}

void LDLModel::ldrawZipUpdated(void)
{
#ifdef HAVE_MINIZIP
	closeZips();
	if (!sm_ldrawZipPath.empty())
	{   
		// LPub3D Mod - unofficial library zip 
		char* unoffZipName = TCUserDefaults::stringForKey("UnoffLDrawZip", "lpub3dldrawunf.zip", false);
		sm_ldrawZip = TCUnzipStream::open(sm_ldrawZipPath.c_str());
		sm_unoffZipPath = directoryFromPath(sm_ldrawZipPath) + "/" + unoffZipName;
		// LPub3D Mod End
		sm_unoffZip = TCUnzipStream::open(sm_unoffZipPath.c_str());
		if (sm_unoffZip == NULL)
		{
			sm_unoffZipPath.clear();
		}
	}
#endif // HAVE_MINIZIP
}

// NOTE: static function.
void LDLModel::setSystemLDrawDir(char* value)
{
	sm_systemLDrawDir = value;
#ifdef WIN32
	if (value != NULL)
	{
		sm_systemLDrawDirSlashes = sm_systemLDrawDir;
		replaceStringCharacter(sm_systemLDrawDirSlashes, '\\', '/');
	}
	else
	{
		sm_systemLDrawDirSlashes.clear();
	}
#endif // WIN32
}

// NOTE: static function.
void LDLModel::setLDrawDir(const char *value)
{
	if (value != sm_systemLDrawDir || !value)
	{
		delete[] sm_systemLDrawDir;
		if (value)
		{
			setSystemLDrawDir(cleanedUpPath(value));
		}
		else
		{
			setSystemLDrawDir(NULL);
		}
		if (sm_lDrawIni)
		{
			LDrawIniFree(sm_lDrawIni);
		}
		sm_lDrawIni = LDrawIniGet(sm_systemLDrawDir, NULL, NULL);
		if (sm_lDrawIni)
		{
			if (fileCaseCallback)
			{
				LDrawIniSetFileCaseCallback(fileCaseCallback);
			}
			if (!sm_systemLDrawDir)
			{
				setSystemLDrawDir(copyString(sm_lDrawIni->LDrawDir));
			}
			if (sm_systemLDrawDir)
			{
				stripTrailingPathSeparators(sm_systemLDrawDir);
				LDrawIniComputeRealDirs(sm_lDrawIni, 1, 0, NULL);
			}
		}
	}
}

void LDLModel::initCheckDirs()
{
	const char *value = getenv("LDRAWDIR");

	if (value)
	{
		sm_checkDirs.push_back(value);
	}
#ifdef WIN32
	char buf[1024];
#if defined(__MINGW64__) || defined (_LP3D_CUI_WGL) || defined (_QT)
	if (GetPrivateProfileStringA("LDraw", "BaseDirectory", "", buf, 1024,
		"ldraw.ini"))
#else
	if (GetPrivateProfileString("LDraw", "BaseDirectory", "", buf, 1024,
		"ldraw.ini"))
#endif
	{
		buf[1023] = 0;
	}
	if (buf[0])
	{
		sm_checkDirs.push_back(buf);
	}
	sm_checkDirs.push_back("C:\\ldraw");
	const char* userProfile = getenv("USERPROFILE");
	if (userProfile != NULL)
	{
		sm_checkDirs.push_back(std::string(userProfile) + "\\ldraw");
	}
#else // WIN32
#ifdef __APPLE__
	const char *libDir = "/Library/ldraw";
	const char *homeDir = getenv("HOME");

	if (homeDir != NULL)
	{
		char *homeLib = copyString(homeDir, strlen(libDir));
		
		stripTrailingPathSeparators(homeLib);
		strcat(homeLib, libDir);
		sm_checkDirs.push_back(homeLib);
		delete[] homeLib;
	}
	sm_checkDirs.push_back(libDir);
	sm_checkDirs.push_back("/Applications/Bricksmith/LDraw");
#else // __APPLE__
	sm_checkDirs.push_back("/usr/share/ldraw");
	const char *homeDir = getenv("HOME");
	if (homeDir != NULL)
	{
		char *cleanHome = cleanedUpPath(homeDir);
		std::string homeLDraw;

		stripTrailingPathSeparators(cleanHome);
		homeLDraw = cleanHome;
		delete[] cleanHome;
		homeLDraw += "/ldraw";
		sm_checkDirs.push_back(homeLDraw);
	}
#endif // __APPLE__
#endif // WIN32
	char *ldviewDir = copyString(TCUserDefaults::getAppPath());
	stripTrailingPathSeparators(ldviewDir);
	char *ldviewLDrawDir = copyString(ldviewDir, 10);

	// LDView Dir/ldraw
	strcat(ldviewLDrawDir, "/ldraw");
	sm_checkDirs.push_back(ldviewLDrawDir);
#ifndef COCOA
	delete[] ldviewLDrawDir;
	char *ldviewParentDir = directoryFromPath(ldviewDir);
	stripTrailingPathSeparators(ldviewParentDir);
	ldviewLDrawDir = copyString(ldviewParentDir, 10);
	delete[] ldviewParentDir;
	// LDView Dir/../ldraw
	strcat(ldviewLDrawDir, "/ldraw");
	sm_checkDirs.push_back(ldviewLDrawDir);
#endif // COCOA
	delete[] ldviewDir;
	delete[] ldviewLDrawDir;
}

const std::string& LDLModel::ldrawZipPath(void)
{
	return sm_ldrawZipPath;
}

// NOTE: static function.
const char* LDLModel::lDrawDir(bool defaultValue /*= false*/)
{
	char *origValue = NULL;

	if (defaultValue)
	{
		if (sm_defaultLDrawDir)
		{
			return sm_defaultLDrawDir;
		}
		origValue = copyString(sm_systemLDrawDir);
		if (sm_systemLDrawDir)
		{
			setLDrawDir(NULL);
		}
	}
	if (!sm_systemLDrawDir)
	{
		bool found = false;

		if (sm_checkDirs.size() == 0)
		{
			initCheckDirs();
		}
		for (StringList::const_iterator it = sm_checkDirs.begin(); !found &&
			 it != sm_checkDirs.end(); ++it)
		{
			const char *dir = it->c_str();

			if (verifyLDrawDir(dir))
			{
				setLDrawDir(dir);
				found = true;
			}
		}
		if (!found)
		{
			setSystemLDrawDir(copyString(""));
		}
	}
	if (defaultValue)
	{
		sm_defaultLDrawDir = copyString(sm_systemLDrawDir);
		setLDrawDir(origValue);
		delete[] origValue;
		return sm_defaultLDrawDir;
	}
	else
	{
		return sm_systemLDrawDir;
	}
}

// NOTE: static function.
std::string LDLModel::unofficialDir(void)
{
	std::string dir(lDrawDir());
	if (dir.empty() && !sm_ldrawZipPath.empty())
	{
		dir = directoryFromPath(sm_ldrawZipPath.c_str());
	}
	if (dir.empty())
	{
		return dir;
	}
	std::string result;
	combinePath(dir.c_str(), "Unofficial", result);
	return result;
}

void LDLModel::readComment(LDLCommentLine *commentLine)
{
	std::string filename;
	std::string author;

	if (commentLine->getMPDFilename(&filename))
	{
		replaceStringCharacter(&filename[0], '\\', '/');
		if (m_flags.mainModelLoaded)
		{
			if (m_activeLineCount == 0)
			{
				m_activeLineCount = commentLine->getLineNumber() - 1;
			}
			if (!getLoadedModels()->objectForKey(filename.c_str()))
			{
				LDLModel *subModel = new LDLModel;

				if (commentLine->isDataMeta())
				{
					subModel->m_dataLine = TCObject::retain(commentLine);
				}
				subModel->setFilename(m_filename);
				initializeNewSubModel(subModel, filename.c_str());
				m_activeMPDModel = subModel;
				if (this == getMainModel())
				{
					getMainModel()->addMpdModel(subModel);
				}
			}
		}
		else
		{
			m_flags.mainModelLoaded = true;
			m_flags.mpd = true;
			if (this == getMainModel())
			{
				getMainModel()->addMpdModel(this);
				if (m_name == NULL)
				{
					setName(filename.c_str());
				}
			}
		}
	}
	else if (commentLine->isPartMeta())
	{
		if (commentLine->isOfficialPartMeta(true))
		{
			// Note that even if we decide it's a primitive, it can still be an
			// official file.
			m_flags.official = true;
		}
		// No matter what the comment says, remember that a primitive cannot
		// be a part, so if we found the file in the P directory, then by
		// definition it isn't a part.
		if (m_flags.mainModelLoaded)
		{
			if (m_activeMPDModel && !m_activeMPDModel->isPrimitive())
			{
				if (m_flags.loadingSubPart)
				{
					m_activeMPDModel->m_flags.subPart = true;
				}
				else if (!m_activeMPDModel->m_flags.subPart)
				{
					m_activeMPDModel->m_flags.part = true;
				}
			}
		}
		else if (!isPrimitive())
		{
			if (m_flags.loadingSubPart)
			{
				m_flags.subPart = true;
			}
			else if (!m_flags.subPart)
			{
				m_flags.part = true;
			}
			// This is now a part, so if we've already hit a BFC CERTIFY line
			// switch it to forced certify.
			if (m_flags.bfcCertify == BFCOnState)
			{
				m_flags.bfcCertify = BFCForcedOnState;
			}
		}
	}
	else if (commentLine->isPrimitiveMeta())
	{
		if (m_flags.mainModelLoaded)
		{
			m_activeMPDModel->m_flags.part = false;
			m_activeMPDModel->m_flags.subPart = false;
			m_activeMPDModel->m_flags.primitive = true;
		}
		else
		{
			m_flags.part = false;
			m_flags.subPart = false;
			m_flags.primitive = true;
		}
	}
	else if (commentLine->isNoShrinkMeta())
	{
		if (m_flags.mainModelLoaded)
		{
			if (m_activeMPDModel)
			{
				m_activeMPDModel->m_flags.noShrink = true;
			}
		}
		else
		{
			m_flags.noShrink = true;
		}
	}
	else if (commentLine->getAuthor(author))
	{
		if (!m_author)
		{
			m_author = copyString(author.c_str());
		}
	}
}

/*
static char *myFgets(char *buf, int bufSize, FILE *file)
{
	int i;

	for (i = 0; i < bufSize - 1; i++)
	{
		int char1 = fgetc(file);

		if (feof(file))
		{
			buf[i] = 0;
			if (i > 0)
			{
				return buf;
			}
			else
			{
				return NULL;
			}
		}
		if (char1 == '\r' || char1 == '\n')
		{
			int char2 = fgetc(file);

			buf[i] = '\n';
			buf[i + 1] = 0;
			if (!feof(file))
			{
				if (!(char1 == '\r' && char2 == '\n' ||
					char1 == '\n' && char2 == '\r'))
				{
					ungetc(char2, file);
				}
			}
			return buf;
		}
		buf[i] = (char)char1;
	}
	buf[bufSize - 1] = 0;
	return buf;
}
*/

void LDLModel::processLine(std::string& line, size_t& lineNumber)
{
	LDLFileLine *fileLine;

	stripCRLF(&line[0]);
	fileLine = LDLFileLine::initFileLine(this, line.c_str(), lineNumber);
	lineNumber++;
	m_fileLines->addObject(fileLine);
	fileLine->release();
	if (fileLine->getLineType() == LDLLineTypeComment)
	{
		// To a certain extent, this will actually parse the comment, but
		// we really need to do some parsing prior to parsing the rest
		// of the file.
		readComment((LDLCommentLine *)fileLine);
	}
}

std::basic_istream<char, std::char_traits<char>>& LDLModel::getLine(std::ifstream &stream, TCUnzipStream *zipStream, std::string& line)
{
	if (zipStream != NULL && zipStream->is_valid())
	{
		return std::getline(*zipStream, line);
	}
	return std::getline(stream, line);
}

bool LDLModel::read(std::ifstream &stream, TCUnzipStream *zipStream)
{
	std::string line;
	size_t lineNumber = 1;
	bool done = false;
	bool retValue = true;

	m_fileLines = new LDLFileLineArray;
	if (isMainModel() && m_mainModel->getHaveCustomConfig())
	{
		std::string line0 = "1 16 0 0 0 1 0 0 0 1 0 0 0 1 ";
		line0 += m_mainModel->getCustomConfigPath();
		lineNumber = 0;
		processLine(line0, lineNumber);
	}
	while (!done && !getLoadCanceled())
	{
		if (getLine(stream, zipStream, line))
		{
			processLine(line, lineNumber);
		}
		else
		{
			if (m_activeLineCount == 0)
			{
				m_activeLineCount = m_fileLines->getCount();
			}
			done = true;
		}
	}
	if (stream.is_open())
	{
		stream.close();
	}
	m_activeMPDModel = NULL;
	return retValue && !getLoadCanceled();
}

void LDLModel::reportProgress(const char *message, float progress,
							  bool mainOnly)
{
	if (!mainOnly || this == m_mainModel)
	{
		bool loadCanceled;

		TCProgressAlert::send("LDLModel", message, progress, &loadCanceled,
			this);
		if (loadCanceled)
		{
			cancelLoad();
		}
	}
}

void LDLModel::reportProgress(const wchar_t *message, float progress,
							  bool mainOnly)
{
	if (!mainOnly || this == m_mainModel)
	{
		bool loadCanceled;

		TCProgressAlert::send("LDLModel", message, progress, &loadCanceled,
			this);
		if (loadCanceled)
		{
			cancelLoad();
		}
	}
}

bool LDLModel::load(
	std::ifstream &stream,
	TCUnzipStream *zipStream,
	bool trackProgress)
{
	bool retValue;

	if (trackProgress)
	{
		reportProgress(LOAD_MESSAGE, 0.0f);
	}
	if (!read(stream, zipStream))
	{
		if (trackProgress)
		{
			reportProgress(LOAD_MESSAGE, 1.0f);
		}
		return false;
	}
	if (trackProgress)
	{
		reportProgress(LOAD_MESSAGE, MAIN_READ_FRACTION);
	}
	retValue = parse();
	if (trackProgress)
	{
		reportProgress(LOAD_MESSAGE, 1.0f);
	}
	return retValue;
}

ptrdiff_t LDLModel::parseMPDMeta(size_t index, const char *filename)
{
	size_t i = index + 1;
	size_t count = m_fileLines->getCount();

	if (m_flags.mainModelParsed)
	{
		LDLModel *subModel;

		for (i = index + 1; i < count; i++)
		{
			LDLFileLine *fileLine = (*m_fileLines)[i];

			if (fileLine->getLineType() == LDLLineTypeComment &&
				((LDLCommentLine *)fileLine)->getMPDFilename())
			{
				break;
			}
		}
		count = i;
		subModel = (LDLModel *)getLoadedModels()->objectForKey(filename);
		if (subModel)
		{
			if (!subModel->m_fileLines)
			{
				subModel->m_fileLines = new LDLFileLineArray(count - index - 1);
				for (i = index + 1; i < count; i++)
				{
					LDLFileLine *fileLine = (*m_fileLines)[i];

					subModel->m_fileLines->addObject(fileLine);
				}
				subModel->m_activeLineCount = subModel->m_fileLines->getCount();
				LDLModel *oldActiveMpd = m_mainModel->m_activeMPDModel;
				m_mainModel->m_activeMPDModel = this;
				if (!subModel->parse())
				{
					m_mainModel->m_activeMPDModel = oldActiveMpd;
					return -1;
				}
				m_mainModel->m_activeMPDModel = oldActiveMpd;
			}
			else
			{
				reportError(LDLEMPDError, *(*m_fileLines)[index],
					TCLocalStrings::get(_UC("LDLModelMpdAlreadyLoaded")));
			}
		}
	}
	else
	{
		m_flags.mainModelParsed = true;
	}
	return i - index - 1;
}

ptrdiff_t LDLModel::parseBBoxIgnoreMeta(LDLCommentLine *commentLine)
{
	if (commentLine->containsBBoxIgnoreCommand("BEGIN"))
	{
		m_flags.bboxIgnoreOn = true;
		m_flags.bboxIgnoreBegun = true;
	}
	else if (commentLine->containsBBoxIgnoreCommand("NEXT"))
	{
		m_flags.bboxIgnoreOn = true;
	}
	else if (commentLine->containsBBoxIgnoreCommand("END"))
	{
		if (!m_flags.bboxIgnoreBegun)
		{
			reportWarning(LDLEMetaCommand, *commentLine,
				TCLocalStrings::get(_UC("LDLModelBBoxEndUnexpected")));
		}
		m_flags.bboxIgnoreOn = false;
		m_flags.bboxIgnoreBegun = false;
	}
	else
	{
		reportWarning(LDLEMetaCommand, *commentLine,
			TCLocalStrings::get(_UC("LDLModelBBoxCommand")));
	}
	return 0;
}

void LDLModel::endTexmap(void)
{
	m_flags.texmapStarted = false;
	m_flags.texmapNext = false;
	TCObject::release(m_texmapImage);
	m_texmapImage = NULL;
}

bool LDLModel::openTexmap(
	const char *filename,
	std::ifstream &texmapStream,
	TCUnzipStream *zipStream,
	std::string &path)
{
	if (!openSubModelNamed(filename, path, texmapStream, zipStream, false, NULL, false))
	{
		LDLFindFileAlert *alert = new LDLFindFileAlert(filename);

		TCAlertManager::sendAlert(alert, this);
		if (alert->getFileFound())
		{
			path = alert->getFilename();
			openStream(path.c_str(), texmapStream);
		}
		alert->release();
	}
	return (texmapStream.is_open() && !texmapStream.fail()) ||
		(zipStream != NULL && zipStream->is_valid());
}

void LDLModel::extractData()
{
	std::string base64Text;
	size_t lineCount = m_fileLines->getCount();
	for (unsigned int i = 0; i < lineCount; ++i)
	{
		LDLFileLine *fileLine = (*m_fileLines)[i];
		if (fileLine->getLineType() == LDLLineTypeComment)
		{
			LDLCommentLine *commentLine = (LDLCommentLine *)fileLine;
			if (commentLine->isDataRowMeta())
			{
				const char *line = commentLine->getLine();
				std::string base64Line;
				base64Line.reserve(strlen(line));
				for (const char *lineChar = strchr(line, ':') + 1; *lineChar; ++lineChar)
				{
					if (isInBase64Charset(*lineChar))
					{
						base64Line += *lineChar;
					}
				}
				base64Text += base64Line;
			}
		}
	}
	if (!base64Decode(base64Text, m_data))
	{
		reportError(LDLEMetaCommand, *m_dataLine,
			TCLocalStrings::get(_UC("LDLModelDataDecodeError")));
	}
}

ptrdiff_t LDLModel::parseTexmapMeta(LDLCommentLine *commentLine)
{
	if (m_flags.texmapStarted && m_flags.texmapNext)
	{
		reportError(LDLEGeneral, *commentLine,
			TCLocalStrings::get(_UC("LDLModelTexmapCommandAfterNext")));
		endTexmap();
	}
	if (m_flags.texmapStarted)
	{
		if (commentLine->containsTexmapCommand("FALLBACK"))
		{
			if (m_flags.texmapFallback)
			{
				reportError(LDLEGeneral, *commentLine,
					TCLocalStrings::get(_UC("LDLModelTexmapMultipleFallback")));
			}
			else
			{
				if (m_texmapFilename.size() > 0)
				{
					// If the texmap image load failed, we need to display the
					// fallback geometry, so don't go into fallback mode.
					m_flags.texmapFallback = true;
				}
			}
		}
		else if (commentLine->containsTexmapCommand("END"))
		{
			endTexmap();
			if (!m_flags.texmapValid)
			{
				commentLine->setValid(false);
			}
		}
		else
		{
			reportError(LDLEMetaCommand, *commentLine,
				TCLocalStrings::get(_UC("LDLModelTexmapUnexpectedCommand")));
		}
	}
	else
	{
		bool isStart = commentLine->containsTexmapCommand("START");
		bool isNext = commentLine->containsTexmapCommand("NEXT");

		if (isStart || isNext)
		{
			const char *typeName = commentLine->getWord(2);
			size_t extraParams = 0;

			if (typeName == NULL) {
				reportError(LDLEParse, *commentLine,
					TCLocalStrings::get(_UC("LDLModelTexmapParseError")));
				commentLine->setValid(false);
				return -1;
			}
			else if (strcmp(typeName, "PLANAR") == 0)
			{
				m_texmapType = LDLFileLine::TTPlanar;
			}
			else if (strcmp(typeName, "CYLINDRICAL") == 0)
			{
				m_texmapType = LDLFileLine::TTCylindrical;
				extraParams = 1;
			}
			else if (strcmp(typeName, "SPHERICAL") == 0)
			{
				m_texmapType = LDLFileLine::TTSpherical;
				extraParams = 2;
			}
			else
			{
				reportError(LDLEGeneral, *commentLine,
					TCLocalStrings::get(_UC("LDLModelTexmapUnknownMethod")));
				commentLine->setValid(false);
				return -1;
			}
			if (commentLine->getNumWords() < 13 + extraParams)
			{
				reportError(LDLEParse, *commentLine,
					TCLocalStrings::get(_UC("LDLModelTexmapParseError")));
				commentLine->setValid(false);
				return -1;
			}
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					m_texmapPoints[i][j] =
						(TCFloat)atof(commentLine->getWord(3 + i * 3 + j));
				}
			}
			for (size_t i = 0; i < extraParams; ++i)
			{
				m_texmapExtra[i] = (TCFloat)atof(commentLine->getWord(12 + i));
			}
			m_flags.texmapStarted = true;
			m_flags.texmapFallback = false;
			m_flags.texmapValid = true;
			if (isNext)
			{
				m_flags.texmapNext = true;
			}
			// Only load the texture map file if textures are enabled.  Still
			// perform the parsing of everything else either way, but don't
			// load the file when they're disabled.
			if (m_mainModel->getTexmaps())
			{
				std::string filename = commentLine->getWord(12 + extraParams);
				std::string pathFilename = std::string("textures/") + filename;
				bool delayedLoad = false;
				std::string path;
				TCDictionary* subModelDict = getLoadedModels();
				LDLModel *texmapModel = (LDLModel*)subModelDict->objectForKey(filename.c_str());
				if (texmapModel != NULL)
				{
					LDLModel *activeMpd = m_mainModel->m_activeMPDModel;
					if (activeMpd != NULL && activeMpd->m_filename != NULL)
					{
						char *baseName = filenameFromPath(activeMpd->m_filename);
						combinePathParts(path, baseName, ":", filename);
						delete[] baseName;
					}
					else
					{
						combinePathParts(path, "MPD:", filename);
					}
					if (texmapModel->m_data.empty())
					{
						if (m_mpdTexmapModels == NULL)
						{
							m_mpdTexmapModels = new LDLModelArray;
							m_mpdTexmapLines = new LDLCommentLineArray;
							m_mpdTexmapImages = new TCImageArray;
						}
						m_mpdTexmapModels->addObject(texmapModel);
						m_mpdTexmapLines->addObject(commentLine);
						m_mainModel->setHaveMpdTexmaps();
						delayedLoad = true;
					}
				}
				std::ifstream texmapStream;
				TCUnzipStream zipStream;
				if (texmapModel == NULL)
				{
					if (!openTexmap(pathFilename.c_str(), texmapStream, &zipStream, path))
					{
						openTexmap(filename.c_str(), texmapStream, &zipStream, path);
					}
				}
				if ((texmapStream.is_open() && !texmapStream.fail()) ||
					zipStream.is_valid() || texmapModel != NULL)
				{
					TCImage *image = new TCImage;
					bool loaded = false;

					image->setLineAlignment(4);
					if (delayedLoad)
					{
						m_mpdTexmapImages->addObject(image);
					}
					else if (texmapModel != NULL)
					{
						loaded = image->loadData(&texmapModel->m_data[0],
							texmapModel->m_data.size());
					}
					else
					{
						if (zipStream.is_valid())
						{
							loaded = image->loadFile(zipStream);
						}
#ifdef DEBUG
						// Loading using stdio fread is PROBABLY slightly faster
						// than loading from a stream, so only do that in debug
						// builds to make sure the istream-based loading works.
						// The standard LDraw zips do not contain any JPG or BMP
						// textures, so the only way to test the streamed
						// loading is with file system-based texture.
						if (!loaded && texmapStream.is_open())
						{
							loaded = image->loadFile(texmapStream);
						}
#endif // DEBUG
						if (texmapStream.is_open())
						{
							texmapStream.close();
						}
						if (!loaded)
						{
							loaded = image->loadFile(path.c_str());
						}
					}
					if (loaded || delayedLoad)
					{
						char *cleanPath = cleanedUpPath(path.c_str());

						m_texmapImage = image;
						convertStringToLower(cleanPath);
						// Since the path is going to be used as a key in a map,
						// we want it to be consistent.  Hence, cleaning it up
						// and making it all lower case.  Files in LDraw cannot
						// have case-sensitive filenames, so this should be
						// kosher.
						m_texmapFilename = cleanPath;
						delete[] cleanPath;
					}
					else
					{
						image->release();
						reportError(LDLEMetaCommand, *commentLine,
							TCLocalStrings::get(_UC("LDLModelTexmapImageLoadError")));
					}
				}
				else
				{
					reportError(LDLEMetaCommand, *commentLine,
						TCLocalStrings::get(_UC("LDLModelTexmapFileNotFound")));
				}
				if (m_texmapImage == NULL)
				{
					commentLine->setValid(false);
					m_flags.texmapValid = false;
				}
			}
		}
		else
		{
			reportError(LDLEMetaCommand, *commentLine,
				TCLocalStrings::get(_UC("LDLModelTexmapUnexpectedCommand")));
		}
	}
	return 0;
}

int LDLModel::loadMpdTexmaps(void)
{
	if (m_mpdTexmapModels == NULL)
	{
		return 0;
	}
	size_t count = m_mpdTexmapModels->getCount();
	for (size_t i = 0; i < count; ++i)
	{
		LDLModel *texmapModel = (*m_mpdTexmapModels)[i];
		TCImage *image = (*m_mpdTexmapImages)[i];
		if (!image->loadData(&texmapModel->m_data[0],
			texmapModel->m_data.size()))
		{
			LDLCommentLine *commentLine = (*m_mpdTexmapLines)[i];
			reportError(LDLEMetaCommand, *commentLine,
				TCLocalStrings::get(_UC("LDLModelTexmapImageLoadError")));
		}
	}
	m_mpdTexmapModels->release();
	m_mpdTexmapModels = NULL;
	m_mpdTexmapLines->release();
	m_mpdTexmapLines = NULL;
	m_mpdTexmapImages->release();
	m_mpdTexmapImages = NULL;
	return 0;
}

ptrdiff_t LDLModel::parseBFCMeta(LDLCommentLine *commentLine)
{
	if (m_flags.bfcInvertNext)
	{
		reportError(LDLEBFCError, *commentLine,
			TCLocalStrings::get(_UC("LDLModelBfcInvert")));
		m_flags.bfcInvertNext = false;
	}
	if (m_flags.bfcCertify == BFCUnknownState)
	{
		if (commentLine->containsBFCCommand("NOCERTIFY"))
		{
			m_flags.bfcCertify = BFCOffState;
			if (m_flags.started)
			{
				reportError(LDLEBFCError, *commentLine,
					TCLocalStrings::get(_UC("LDLModelBfcNoCertFirst")));
			}
		}
		else
		{
			if (m_flags.started)
			{
				m_flags.bfcCertify = BFCOffState;
				reportError(LDLEBFCError, *commentLine,
					TCLocalStrings::get(_UC("LDLModelBfcFirst")));
			}
			else
			{
				// Unless a NOCERTIFY is present, CERTIFY gets turned on by
				// default for any BFC command.
				if (m_flags.part || m_flags.subPart)
				{
					// BFC certified parts force BFC to be on, even if their
					// parent models don't have BFC certification.
					m_flags.bfcCertify = BFCForcedOnState;
				}
				else
				{
					m_flags.bfcCertify = BFCOnState;
				}
				m_flags.bfcClip = true;
			}
		}
	}
	else
	{
		if (commentLine->containsBFCCommand("CERTIFY"))
		{
			if (getBFCOn())
			{
				reportWarning(LDLEBFCWarning, *commentLine,
					TCLocalStrings::get(_UC("LDLModelBfcCertNotFirst")));
			}
			else
			{
				reportError(LDLEBFCError, *commentLine,
					TCLocalStrings::get(_UC("LDLModelBfcCertNoCert")));
			}
		}
		// Not else if below, because each BFC command could potentially
		// have both certify AND nocertify.
		if (commentLine->containsBFCCommand("NOCERTIFY"))
		{
			if (getBFCOn())
			{
				reportError(LDLEBFCError, *commentLine,
					TCLocalStrings::get(_UC("LDLModelBfcNoCertCert")));
			}
			else
			{
				reportWarning(LDLEBFCWarning, *commentLine,
					TCLocalStrings::get(_UC("LDLModelBfcNoCertMulti")));
			}
		}
	}
	if (getBFCOn())
	{
		if (commentLine->containsBFCCommand("CLIP"))
		{
			if (commentLine->containsBFCCommand("NOCLIP"))
			{
				reportError(LDLEBFCError, *commentLine,
					TCLocalStrings::get(_UC("LDLModelBfcClipNoClip")));
			}
			else
			{
				m_flags.bfcClip = true;
			}
		}
		else if (commentLine->containsBFCCommand("NOCLIP"))
		{
			m_flags.bfcClip = false;
		}
		if (commentLine->containsBFCCommand("CCW"))
		{
			if (commentLine->containsBFCCommand("CW"))
			{
				reportError(LDLEBFCError, *commentLine,
					TCLocalStrings::get(_UC("LDLModelBfcCwCcw")));
			}
			else
			{
				m_flags.bfcWindingCCW = true;
			}
		}
		else if (commentLine->containsBFCCommand("CW"))
		{
			m_flags.bfcWindingCCW = false;
		}
		if (commentLine->containsBFCCommand("INVERTNEXT"))
		{
			m_flags.bfcInvertNext = true;
		}
	}
	else
	{
		if (commentLine->containsBFCCommand("CLIP") ||
			commentLine->containsBFCCommand("NOCLIP") ||
			commentLine->containsBFCCommand("CW") ||
			commentLine->containsBFCCommand("CCW") ||
			commentLine->containsBFCCommand("INVERTNEXT"))
		{
			reportError(LDLEBFCError, *commentLine,
				TCLocalStrings::get(_UC("LDLModelBfcAfterNoCert")));
		}
	}
	return 0;
}

ptrdiff_t LDLModel::parseComment(size_t index, LDLCommentLine *commentLine)
{
	std::string filename;

	if (commentLine->getMPDFilename(&filename))
	{
		replaceStringCharacter(&filename[0], '\\', '/');
		return parseMPDMeta(index, filename.c_str());
	}
	else if (commentLine->isBFCMeta())
	{
		return parseBFCMeta(commentLine);
	}
	else if (commentLine->isPartMeta())
	{
		m_stepIndices.push_back(index);
		return 0;
	}
	else if (commentLine->isLDViewMeta())
	{
		if (commentLine->isBBoxIgnoreMeta())
		{
			return parseBBoxIgnoreMeta(commentLine);
		}
		else
		{
			reportWarning(LDLEMetaCommand, *commentLine,
				TCLocalStrings::get(_UC("LDLModelUnknownLDViewMeta")));
		}
	}
	else if (commentLine->isTexmapMeta())
	{
		return parseTexmapMeta(commentLine);
	}
	else if (index == 0)
	{
		delete[] m_description;
		m_description = copyString(&commentLine->getLine()[1]);
		stripLeadingWhitespace(m_description);
		stripTrailingWhitespace(m_description);
	}
	return 0;
}

/*
void LDLModel::processModelLine(LDLModelLine *modelLine)
{
	m_flags.started = true;
}
*/

bool LDLModel::parse(void)
{
	if (m_fileLines)
	{
		if (m_dataLine != NULL)
		{
			extractData();
			// Note: even if extractData fails, that does NOT mean that we
			// failed to parse the file.
			return true;
		}
		size_t i;
		size_t count = m_fileLines->getCount();

		// ********************************************************************
		// NOTE: This for loop does a number of things that aren't normally
		// done (at least by me).  In one place (when a line needs to be
		// replaced by new ones), it inserts new items in the array just after
		// the current spot, changes count, and uses continue.  In another place
		// (when it sees an MPD secondary file), it increases i to skip over all
		// the lines in that secondary file (they get parsed separately).
		// ********************************************************************
		for (i = 0; i < count && !getLoadCanceled(); i++)
		{
			LDLFileLine *fileLine = (*m_fileLines)[i];
			bool checkInvertNext = true;

			if (fileLine->isActionLine())
			{
				LDLActionLine *actionLine = (LDLActionLine *)fileLine;

				if (actionLine->getLineNumber() != 0)
				{
					// Line number 0 is the custom config model, which doesn't
					// count as starting the model, even if it contains
					// geometry.
					m_flags.started = true;
				}
				actionLine->setBFCSettings(m_flags.bfcCertify, m_flags.bfcClip,
					m_flags.bfcWindingCCW, m_flags.bfcInvertNext);
				if (m_flags.bboxIgnoreOn)
				{
					actionLine->setBBoxIgnore(true);
					m_mainModel->setBBoxIgnoreUsed(true);
				}
				if (!m_flags.bboxIgnoreBegun)
				{
					m_flags.bboxIgnoreOn = false;
				}
				if (m_flags.texmapStarted)
				{
					if (m_flags.texmapFallback)
					{
						actionLine->setTexmapFallback();
					}
					else
					{
						actionLine->setTexmapSettings(m_texmapType,
							m_texmapFilename, m_texmapImage, m_texmapPoints,
							m_texmapExtra);
						if (m_flags.texmapNext)
						{
							endTexmap();
						}
					}
				}
			}
			else
			{
				checkInvertNext = false;
				if (fileLine->getLineType() == LDLLineTypeComment &&
					m_flags.texmapStarted && !m_flags.texmapFallback)
				{
					((LDLCommentLine *)fileLine)->setTexmapSettings(
						m_texmapType, m_texmapFilename, m_texmapImage,
						m_texmapPoints, m_texmapExtra);
				}
			}
			if (fileLine->parse())
			{
				if (fileLine->isValid())
				{
					if (fileLine->getError())
					{
						sendAlert(fileLine->getError());
					}
				}
				else
				{
					LDLFileLineArray *replacementLines =
						fileLine->getReplacementLines();

					if (replacementLines)
					{
						size_t replacementCount = replacementLines->getCount();
						size_t j;

						fileLine->setReplaced(true);
						for (j = 0; j < replacementCount; j++)
						{
							m_fileLines->insertObject((*replacementLines)[j],
								i + 1);
						}
						// Note that if we do get here, we haven't gotten past
						// m_activeLineCount, because that parsing gets done in
						// the secondary files themselves.
						m_activeLineCount += replacementCount;
						count += replacementCount;
						if (fileLine->getError())
						{
							sendAlert(fileLine->getError());
						}
						replacementLines->release();
						// ****************************************************
						// Note the use of continue below.  I really shy away
						// from using it, but I'm goint to do so here.
						// ****************************************************
						continue;
						// ****************************************************
						// ****************************************************
					}
					else
					{
						sendAlert(fileLine->getError());
					}
				}
			}
			else
			{
				sendAlert(fileLine->getError());
			}
			switch (fileLine->getLineType())
			{
			case LDLLineTypeComment:
				{
					ptrdiff_t skippedLines = parseComment(i,
						(LDLCommentLine *)fileLine);

					checkInvertNext = false;
					if (skippedLines >= 0)
					{
						// ****************************************************
						// Note that I increment i below to skip over the lines
						// in the MPD secondary file I just encountered.  (The
						// parseComment function will only return a number
						// greater than 0 if it found an MPD secondary file.
						// ****************************************************
						i += skippedLines;
						// ****************************************************
						// ****************************************************
					}
				}
				break;
			case LDLLineTypeModel:
				{
					LDLModel *model = ((LDLModelLine *)fileLine)->getModel();

					if (model)
					{
						model->calcBoundingBox();
					}
					if (model != NULL && model->hasStuds())
					{
						m_flags.hasStuds = true;
					}
					m_flags.bfcInvertNext = false;
				}
				break;
			default:
				break;
			}
			if (checkInvertNext && m_flags.bfcInvertNext)
			{
				reportError(LDLEBFCError, *fileLine,
					TCLocalStrings::get(_UC("LDLModelBfcInvert")));
				m_flags.bfcInvertNext = false;
			}
			fileLine->setStepIndex((int)m_stepIndices.size());
			reportProgress(LOAD_MESSAGE, (float)i / (float)m_activeLineCount *
				(1.0f - MAIN_READ_FRACTION) + MAIN_READ_FRACTION);
			// If a primitive is in a user model, it will prevent parts in the
			// file after the primitive from being detected as parts if we don't
			// reset the loadingPrimitive flag after each file line. The others
			// are reset because they might also cause different problems.
			m_flags.loadingPart = false;
			m_flags.loadingSubPart = false;
			m_flags.loadingUnoffic = false;
			m_flags.loadingPrimitive = false;
		}
		return !getLoadCanceled();
	}
	else
	{
		// This is in an MPD, and has not yet been fully initialized.
		return true;
	}
}

void LDLModel::cancelLoad(void)
{
	m_mainModel->cancelLoad();
}

bool LDLModel::getLoadCanceled(void)
{
	return m_mainModel->getLoadCanceled();
}

void LDLModel::sendAlert(LDLError *alert)
{
	if (alert)
	{
		TCAlertManager::sendAlert(alert, this);
		if (alert->getLoadCanceled())
		{
			cancelLoad();
		}
	}
}

/*
void LDLModel::sendAlert(LDLErrorType type, LDLAlertLevel level,
						 const char* format, va_list argPtr)
{
	LDLError *alert;

	alert = newError(type, format, argPtr);
	alert->setLevel(level);
	sendAlert(alert);
	alert->release();
}
*/

void LDLModel::sendAlert(LDLErrorType type, LDLAlertLevel level, CUCSTR format,
						 va_list argPtr)
{
	LDLError *alert;

	alert = newError(type, format, argPtr);
	alert->setLevel(level);
	sendAlert(alert);
	alert->release();
}

/*
void LDLModel::sendAlert(LDLErrorType type, LDLAlertLevel level,
						 const LDLFileLine &fileLine, const char* format,
						 va_list argPtr)
{
	LDLError *alert;

	alert = newError(type, fileLine, format, argPtr);
	alert->setLevel(level);
	sendAlert(alert);
	alert->release();
}
*/

void LDLModel::sendAlert(LDLErrorType type, LDLAlertLevel level,
						 const LDLFileLine &fileLine, CUCSTR format,
						 va_list argPtr)
{
	LDLError *alert;

	alert = newError(type, fileLine, format, argPtr);
	alert->setLevel(level);
	sendAlert(alert);
	alert->release();
}

/*
void LDLModel::reportError(LDLErrorType type, const LDLFileLine &fileLine,
						   const char* format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	sendAlert(type, LDLAError, fileLine, format, argPtr);
	va_end(argPtr);
}
*/

void LDLModel::reportError(LDLErrorType type, const LDLFileLine &fileLine,
						   CUCSTR format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	sendAlert(type, LDLAError, fileLine, format, argPtr);
	va_end(argPtr);
}

/*
void LDLModel::reportWarning(LDLErrorType type, const LDLFileLine &fileLine,
							 const char* format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	sendAlert(type, LDLAWarning, fileLine, format, argPtr);
	va_end(argPtr);
}
*/

void LDLModel::reportWarning(LDLErrorType type, const LDLFileLine &fileLine,
							 CUCSTR format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	sendAlert(type, LDLAWarning, fileLine, format, argPtr);
	va_end(argPtr);
}

/*
void LDLModel::reportError(LDLErrorType type, const char* format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	sendAlert(type, LDLAError, format, argPtr);
	va_end(argPtr);
}
*/

void LDLModel::reportError(LDLErrorType type, CUCSTR format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	sendAlert(type, LDLAError, format, argPtr);
	va_end(argPtr);
}

/*
void LDLModel::reportWarning(LDLErrorType type, const char* format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	sendAlert(type, LDLAWarning, format, argPtr);
	va_end(argPtr);
}
*/

void LDLModel::reportWarning(LDLErrorType type, CUCSTR format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	sendAlert(type, LDLAWarning, format, argPtr);
	va_end(argPtr);
}

TCDictionary *LDLModel::getLoadedModels(void)
{
	return (m_mainModel->getLoadedModels());
}

void LDLModel::print(int indent) const
{
	indentPrintf(indent, "LDLModel");
	if (m_flags.part)
	{
		printf(" (Part)");
	}
	if (m_flags.subPart)
	{
		printf(" (SubPart)");
	}
	switch (m_flags.bfcCertify)
	{
	case BFCOffState:
		printf(" (BFC Off)");
		break;
	case BFCOnState:
		printf(" (BFC On)");
		break;
	case BFCForcedOnState:
		printf(" (BFC Forced On)");
		break;
	default:
		break;
	}
	printf(": ");
	if (m_filename)
	{
		printf("%s", m_filename);
	}
	else
	{
		printf("<Unknown>");
	}
	printf("\n");
	if (m_fileLines)
	{
		size_t i;
		size_t count = m_fileLines->getCount();

		for (i = 0; i < count; i++)
		{
			(*m_fileLines)[i]->print(indent + 1);
		}
	}
}

bool LDLModel::getLowResStuds(void) const
{
	return m_mainModel->getLowResStuds();
}

LDLError *LDLModel::newError(LDLErrorType type, const LDLFileLine &fileLine,
							 CUCSTR format, va_list argPtr)
{
	UCCHAR message[1024];
	UCCHAR** components;
	size_t componentCount;
	LDLError *error = NULL;

	vsucprintf(message, COUNT_OF(message), format, argPtr);
	stripCRLF(message);
	components = componentsSeparatedByString(message, _UC("\n"),
		componentCount);
	if (componentCount > 1)
	{
		size_t i;
#ifdef TC_NO_UNICODE
		TCStringArray *extraInfo = new TCStringArray(componentCount - 1);

		*strchr(message, '\n') = 0;
		for (i = 1; i < componentCount; i++)
		{
			extraInfo->addString(components[i]);
		}
		error = new LDLError(type, message, m_filename, fileLine,
			fileLine.getFormattedLine(), fileLine.getLineNumber(), extraInfo);
		extraInfo->release();
#else // TC_NO_UNICODE
		ucstringVector extraInfo;
		wchar_t *newLine = wcschr(message, '\n');
		if (newLine != NULL)
		{
			*newLine = 0;
		}
		extraInfo.reserve((size_t)componentCount - 1);
		for (i = 1; i < componentCount; i++)
		{
			extraInfo.push_back(components[i]);
		}
		error = new LDLError(type, message, m_filename, fileLine,
			fileLine.getFormattedLine(), fileLine.getLineNumber(), extraInfo);
#endif // TC_NO_UNICODE
	}
	else
	{
		error = new LDLError(type, message, m_filename, fileLine,
			fileLine.getFormattedLine(), fileLine.getLineNumber());
	}
	deleteStringArray(components, componentCount);
	return error;
}

LDLError *LDLModel::newError(LDLErrorType type, const LDLFileLine &fileLine,
							 CUCSTR format, ...)
{
	va_list argPtr;
	LDLError *retValue;

	va_start(argPtr, format);
	retValue = newError(type, fileLine, format, argPtr);
	va_end(argPtr);
	return retValue;
}

LDLError *LDLModel::newError(LDLErrorType type, CUCSTR format, va_list argPtr)
{
	UCCHAR message[1024];
	UCCHAR** components;
	size_t componentCount;
	LDLError *error = NULL;

	vsucprintf(message, COUNT_OF(message), format, argPtr);
	stripCRLF(message);
	components = componentsSeparatedByString(message, _UC("\n"),
		componentCount);
	if (componentCount > 1)
	{
		size_t i;
#ifdef TC_NO_UNICODE
		TCStringArray *extraInfo = new TCStringArray(componentCount - 1);

		*strchr(message, '\n') = 0;
		for (i = 1; i < componentCount; i++)
		{
			extraInfo->addString(components[i]);
		}
		error = new LDLError(type, message, m_filename, NULL, NULL, -1,
			extraInfo);
		extraInfo->release();
#else // TC_NO_UNICODE
		ucstringVector extraInfo;
		wchar_t* newLine = wcschr(message, '\n');
		if (newLine != NULL)
		{
			*newLine = 0;
		}
		extraInfo.reserve((size_t)componentCount - 1);
		for (i = 1; i < componentCount; i++)
		{
			extraInfo.push_back(components[i]);
		}
		error = new LDLError(type, message, m_filename, NULL, NULL, -1,
			extraInfo);
#endif // TC_NO_UNICODE
	}
	else
	{
		error = new LDLError(type, message, m_filename, NULL, NULL, -1);
	}
	deleteStringArray(components, componentCount);
	return error;
}

LDLError *LDLModel::newError(LDLErrorType type, CUCSTR format, ...)
{
	va_list argPtr;
	LDLError *retValue;

	va_start(argPtr, format);
	retValue = newError(type, format, argPtr);
	va_end(argPtr);
	return retValue;
}

void LDLModel::scanPoints(
	TCObject *scanner,
	LDLScanPointCallback scanPointCallback,
	const TCFloat *matrix,
	ptrdiff_t step /*= -1*/,
	bool watchBBoxIgnore /*= false*/,
	LDLStatistics *statistics /*= NULL*/) const
{
	if (this != m_mainModel && isPart() && m_mainModel->getBoundingBoxesOnly()
		&& m_flags.haveBoundingBox)
	{
		TCVector boxPoints[8];
		TCVector point;
		int i;

		boxPoints[0] = m_boundingMin;
		boxPoints[4] = m_boundingMax;
		// Bottom square
		boxPoints[1] = boxPoints[0];
		boxPoints[1][0] = boxPoints[4][0];
		boxPoints[2] = boxPoints[0];
		boxPoints[2][1] = boxPoints[4][1];
		boxPoints[3] = boxPoints[4];
		boxPoints[3][2] = boxPoints[0][2];
		// Top square
		boxPoints[5] = boxPoints[4];
		boxPoints[5][0] = boxPoints[0][0];
		boxPoints[6] = boxPoints[4];
		boxPoints[6][1] = boxPoints[0][1];
		boxPoints[7] = boxPoints[0];
		boxPoints[7][2] = boxPoints[4][2];
		for (i = 0; i < 8; i++)
		{
			boxPoints[i].transformPoint(matrix, point);
			((*scanner).*scanPointCallback)(point, NULL);
		}
	}
	else
	{
		int curStep = 0;
		bool emptyStep = true;

		for (unsigned int i = 0; i < m_activeLineCount; i++)
		{
			LDLFileLine *fileLine = (*m_fileLines)[i];
			if (fileLine->getLineNumber() == 0)
			{
				// Line Number 0 is only used by the custom config file,
				// which is not included in point scans.
				continue;
			}
			if (statistics != NULL && !statistics->calculated) {
				fileLine->updateStatistics(*statistics);
			}
			if (step >= 0 && fileLine->getLineType() == LDLLineTypeComment)
			{
				LDLCommentLine *commentLine = (LDLCommentLine *)fileLine;
				
				if (commentLine->isStepMeta() && !emptyStep)
				{
					emptyStep = true;
					if (++curStep > step)
					{
						break;
					}
				}
			}
			if (fileLine->isActionLine())
			{
				LDLActionLine *actionLine = (LDLActionLine *)fileLine;

				emptyStep = false;
				if (!watchBBoxIgnore || !actionLine->getBBoxIgnore())
				{
					if (actionLine->getLineType() == LDLLineTypeModel)
					{
						LDLModelLine *modelLine = (LDLModelLine *)actionLine;
						modelLine->scanPoints(scanner, scanPointCallback,
							matrix, watchBBoxIgnore, statistics);
						if (statistics != NULL)
						{
							LDLModel *model = modelLine->getModel();
							if (model != NULL && model->isPart())
							{
								--statistics->scanningPartDepth;
							}
						}
					}
					else
					{
						actionLine->scanPoints(scanner, scanPointCallback,
							matrix, watchBBoxIgnore);
					}
				}
			}
		}
	}
}

void LDLModel::getBoundingBox(TCVector &min, TCVector &max) const
{
	calcBoundingBox();
	min = m_boundingMin;
	max = m_boundingMax;
}

TCFloat LDLModel::getMaxRadius(
	const TCVector &center,
	bool watchBBoxIgnore,
	LDLStatistics *statistics /*= NULL*/)
{
	calcMaxRadius(center, watchBBoxIgnore, statistics);
	if (watchBBoxIgnore)
	{
		return m_maxRadius;
	}
	else
	{
		return m_maxFullRadius;
	}
}

void LDLModel::scanBoundingBoxPoint(
	const TCVector &point,
	LDLFileLine *pFileLine)
{
	if (pFileLine == NULL ||
		pFileLine->getLineType() != LDLLineTypeConditionalLine)
	{
		if (m_flags.haveBoundingBox)
		{
			for (int i = 0; i < 3; i++)
			{
				if (point[i] < m_boundingMin[i])
				{
					m_boundingMin[i] = point[i];
				}
				else if (point[i] > m_boundingMax[i])
				{
					m_boundingMax[i] = point[i];
				}
			}
		}
		else
		{
			m_boundingMin = m_boundingMax = point;
			m_flags.haveBoundingBox = true;
		}
	}
}

void LDLModel::calcBoundingBox(void) const
{
	if (!m_flags.haveBoundingBox)
	{
		TCFloat matrix[16];

		if (this == m_mainModel && m_mainModel->getBoundingBoxesOnly())
		{
			size_t i;

			for (i = 0; i < m_fileLines->getCount(); i++)
			{
				LDLFileLine *fileLine = (*m_fileLines)[i];

				if (fileLine->getLineNumber() == 0)
				{
					// Line Number 0 is only used by the custom config file,
					// which is not included in bounding box calculations.
					continue;
				}
				if (fileLine->getLineType() == LDLLineTypeModel)
				{
					LDLModelLine *modelLine = (LDLModelLine *)fileLine;
					LDLModel *model = modelLine->getModel();

					if (model != NULL)
					{
						model->calcBoundingBox();
					}
				}
			}
		}
		TCVector::initIdentityMatrix(matrix);
		// NOTE: we cannot compute bounding boxes heirarchically due to
		// rotation of child models.  With their rotation, their bounding
		// boxes can easily stick out of the really minimum bounding box of
		// their parent.
		scanPoints(const_cast<LDLModel *>(this),
			(LDLScanPointCallback)&LDLModel::scanBoundingBoxPoint, matrix, -1, true);
	}
}

void LDLModel::scanRadiusSquaredPoint(
	const TCVector &point,
	LDLFileLine *pFileLine)
{
	if (pFileLine == NULL ||
		pFileLine->getLineType() != LDLLineTypeConditionalLine)
	{
		TCFloat radius = (m_center - point).lengthSquared();

		if (m_flags.fullRadius)
		{
			if (!m_flags.haveMaxFullRadius || radius > m_maxFullRadius)
			{
				m_flags.haveMaxFullRadius = true;
				m_maxFullRadius = radius;
			}
		}
		else
		{
			if (!m_flags.haveMaxRadius || radius > m_maxRadius)
			{
				m_flags.haveMaxRadius = true;
				m_maxRadius = radius;
			}
		}
	}
}

void LDLModel::calcMaxRadius(
	const TCVector &center,
	bool watchBBoxIgnore,
	LDLStatistics *statistics)
{
	if (m_center != center)
	{
		m_flags.haveMaxRadius = false;
		m_flags.haveMaxFullRadius = false;
	}
	if ((watchBBoxIgnore && !m_flags.haveMaxRadius) ||
		(!watchBBoxIgnore && !m_flags.haveMaxFullRadius))
	{
		TCFloat matrix[16];

		TCVector::initIdentityMatrix(matrix);
		m_center = center;
		m_flags.fullRadius = !watchBBoxIgnore;
		if (watchBBoxIgnore)
		{
			m_maxRadius = 0;
		}
		else
		{
			m_maxFullRadius = 0;
		}
		scanPoints(this,
			(LDLScanPointCallback)&LDLModel::scanRadiusSquaredPoint, matrix, -1,
			watchBBoxIgnore, statistics);
		if (watchBBoxIgnore)
		{
			m_maxRadius = (float)sqrt(m_maxRadius);
		}
		else
		{
			m_maxFullRadius = (float)sqrt(m_maxFullRadius);
		}
	}
}

TCObject *LDLModel::getAlertSender(void)
{
	return m_mainModel->getAlertSender();
}

bool LDLModel::hasBoundingBox(void) const
{
	return m_flags.haveBoundingBox != false;
}

LDLFileLineArray *LDLModel::getFileLines(bool initialize /*= false*/)
{
	if (initialize && m_fileLines == NULL)
	{
		m_fileLines = new LDLFileLineArray;
	}
	return m_fileLines;
}

void LDLModel::copyPublicFlags(const LDLModel *src)
{
	m_flags.part = src->m_flags.part;
	m_flags.subPart = src->m_flags.subPart;
	m_flags.primitive = src->m_flags.primitive;
	m_flags.mpd = src->m_flags.mpd;
	m_flags.noShrink = src->m_flags.noShrink;
	m_flags.official = src->m_flags.official;
	m_flags.hasStuds = src->m_flags.hasStuds;
	m_flags.bfcCertify = src->m_flags.bfcCertify;
}

void LDLModel::copyBoundingBox(const LDLModel *src)
{
	if (!src->m_flags.haveBoundingBox)
	{
		src->calcBoundingBox();
	}
	m_boundingMin = src->m_boundingMin;
	m_boundingMax = src->m_boundingMax;
	m_flags.haveBoundingBox = true;
}

bool LDLModel::searchNext(
	const std::string &searchString,
	PtrDiffTVector& path,
	ptrdiff_t loopEnd,
	TCULong activeLineTypes) const
{
	if (m_fileLines == NULL || m_activeLineCount == 0)
	{
		path.clear();
		return false;
	}
	PtrDiffTVector result;
	size_t count = m_activeLineCount;
	ptrdiff_t endIndex = path.empty() && loopEnd != -1 ? loopEnd + 1: count;
	ptrdiff_t startIndex = path.empty() ? 0 : path[0];
	for (ptrdiff_t i = startIndex; i < endIndex; ++i)
	{
		const LDLFileLine *child = (*m_fileLines)[i];
		if (((1 << child->getLineType()) & activeLineTypes) == 0)
		{
			continue;
		}
		PtrDiffTVector childPath;
		int lineOffset = 0;
		bool skipText = false;
		if (!path.empty() && i == path[0])
		{
			skipText = true;
			if (path.size() > 1)
			{
				childPath.insert(childPath.end(), path.begin() + 1, path.end());
			}
		}
		if (!skipText)
		{
			const char *match = strcasestr(child->getLine() + lineOffset,
				searchString.c_str());

			if (match != NULL)
			{
				result.push_back(i);
				path = result;
				return true;
			}
		}
		if (child->getLineType() == LDLLineTypeModel)
		{
			LDLModel *childModel = ((LDLModelLine *)child)->getModel();
			
			if (childModel != NULL && childModel->searchNext(searchString,
				childPath, -1, activeLineTypes))
			{
				result.push_back(i);
				result.insert(result.end(), childPath.begin(), childPath.end());
				path = result;
				return true;
			}
		}
	}
	if (!path.empty() && loopEnd != -1)
	{
		path.clear();
		return searchNext(searchString, path, loopEnd, activeLineTypes);
	}
	path.clear();
	return false;
}

bool LDLModel::searchPrevious(
	const std::string &searchString,
	PtrDiffTVector& path,
	ptrdiff_t loopEnd,
	TCULong activeLineTypes) const
{
	if (m_fileLines == NULL || m_activeLineCount == 0)
	{
		path.clear();
		return false;
	}
	PtrDiffTVector result;
	size_t count = m_activeLineCount;
	ptrdiff_t startIndex = path.empty() ? count - 1 : path[0];
	ptrdiff_t endIndex = path.empty() && loopEnd != -1 ? loopEnd : 0;
	for (ptrdiff_t i = startIndex; i >= endIndex; --i)
	{
		const LDLFileLine *child = (*m_fileLines)[i];
		if (((1 << child->getLineType()) & activeLineTypes) == 0)
		{
			continue;
		}
		PtrDiffTVector childPath;
		int lineOffset = 0;
		if (!path.empty() && i == path[0])
		{
			if (path.size() == 1)
			{
				continue;
			}
			else
			{
				childPath.insert(childPath.end(), path.begin() + 1, path.end());
			}
		}
		if (child->getLineType() == LDLLineTypeModel)
		{
			LDLModel *childModel = ((LDLModelLine *)child)->getModel();
			
			if (childModel != NULL && childModel->searchPrevious(searchString,
				childPath, -1, activeLineTypes))
			{
				result.push_back(i);
				result.insert(result.end(), childPath.begin(), childPath.end());
				path = result;
				return true;
			}
		}
		const char *match = strcasestr(child->getLine() + lineOffset,
			searchString.c_str());

		if (match != NULL)
		{
			result.push_back(i);
			path = result;
			return true;
		}
	}
	if (!path.empty() && loopEnd != -1)
	{
		path.clear();
		return searchPrevious(searchString, path, loopEnd, activeLineTypes);
	}
	path.clear();
	return false;
}
