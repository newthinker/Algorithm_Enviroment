// Reference.h: interface for the CReference class.
//
//////////////////////////////////////////////////////////////////////

#ifndef REFERENCE_H
#define REFERENCE_H

#include <math.h>

#include "BaseDefine.h"
#include "Progress.h"
#include "Output.h"

#define TRANMERC_NO_ERROR           0x0000
#define TRANMERC_LAT_ERROR          0x0001
#define TRANMERC_LON_ERROR          0x0002
#define TRANMERC_EASTING_ERROR      0x0004
#define TRANMERC_NORTHING_ERROR     0x0008
#define TRANMERC_ORIGIN_LAT_ERROR   0x0010
#define TRANMERC_CENT_MER_ERROR     0x0020
#define TRANMERC_A_ERROR            0x0040
#define TRANMERC_B_ERROR            0x0080
#define TRANMERC_A_LESS_B_ERROR     0x0100
#define TRANMERC_SCALE_FACTOR_ERROR 0x0200
#define TRANMERC_LON_WARNING        0x0400

#define UTM_NO_ERROR            0x0000
#define UTM_LAT_ERROR           0x0001
#define UTM_LON_ERROR           0x0002
#define UTM_EASTING_ERROR       0x0004
#define UTM_NORTHING_ERROR      0x0008
#define UTM_ZONE_ERROR          0x0010
#define UTM_HEMISPHERE_ERROR    0x0020
#define UTM_ZONE_OVERRIDE_ERROR 0x0040
#define UTM_A_ERROR             0x0080
#define UTM_B_ERROR             0x0100
#define UTM_A_LESS_B_ERROR      0x0200

#define PI 3.141592653589793238462643383279502884197169399375105820974944592308   /* PI     */
#define PI_OVER         (PI/2.0e0)            /* PI over 2 */
#define MAX_DELTA_LONG  ((PI * 90)/180.0)    /* 90 degrees in radians */
#define MIN_SCALE_FACTOR  0.3
#define MAX_SCALE_FACTOR  3.0
#define MAX_LAT		 ( (84.5 * PI) / 180.0 )
#define MIN_LAT      ( (-80.5 * PI) / 180.0 ) /* -80.5 degrees in radians    */
#define MIN_EASTING  100000
#define MAX_EASTING  900000
#define MIN_NORTHING 0
#define MAX_NORTHING 10000000

#endif // REFERENCE_H
