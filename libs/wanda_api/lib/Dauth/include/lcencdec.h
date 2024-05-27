#ifndef LICCHECKENCDEC_H
#define LICCHECKENCDEC_H

#if HAVE_CONFIG_H
  #include "config.h"
  #define FTN_CALL  /* nothing */
  #define CLOSE_DAT_FILE          FC_FUNC(cldtnf,CLDTNF)
#else
  #ifdef __GNUC__
    #define FTN_CALL __attribute__((cdecl))
  #else
    #define FTN_CALL __cdecl
  #endif
  #define F90_INITIALIZE INITIALIZE
  #define F90_ENABLEBORROW ENABLEBORROW
  #define F90_BORROWTIMELIMIT BORROWTIMELIMIT
  #define F90_MAXBORROWTIME MAXBORROWTIME
  #define F90_DISABLEBORROW DISABLEBORROW
  #define F90_RETURNBORROWEDFEATURE RETURNBORROWEDFEATURE
  #define F90_GETBORROWEDFEATURES GETBORROWEDFEATURES
  #define F90_CHECKOUT CHECKOUT
  #define F90_CHECKIN CHECKIN
  #define F90_GETVENDOR GETVENDOR
  #define F90_GETDISTRIBUTOR GETDISTRIBUTOR
  #define F90_GETNOTICE GETNOTICE
  #define F90_GETUSERNAME GETUSERNAME
  #define F90_GETCOUNT GETCOUNT
  #define F90_GETEXPIRATION GETEXPIRATION
  #define F90_GETCURRENTUSERS GETCURRENTUSERS
  #define F90_GETUSEDLICENSES GETUSEDLICENSES
  #define F90_GETAVAILABLELICENSES GETAVAILABLELICENSES
  #define F90_GETAVAILABLEFEATURES GETAVAILABLEFEATURES
  #define F90_GETNUMBEROFLICENSESPERFEATURE GETNUMBEROFLICENSESPERFEATURE
  #define F90_GETISSERVERPERFEATUREINDEX GETISSERVERPERFEATUREINDEX
  #define F90_GETFEATUREVERSIONPERFEATUREINDEX GETFEATUREVERSIONPERFEATUREINDEX
  #define F90_ISSERVERAVAILABLE ISSERVERAVAILABLE  
  #define F90_ISSERVER ISSERVER  
  #define F90_GETERRORS GETERRORS
  #define F90_GETLASTERROR GETLASTERROR
  #define F90_CLEANUP CLEANUP
  #define F90_GETFEATUREVERSION GETFEATUREVERSION
  #define F90_GETAVAILABLELICENSESPERFEATUREINDEXCOUNT GETAVAILABLELICENSESPERFEATUREINDEXCOUNT
  #define F90_GETUSEDLICENSESPERFEATUREINDEXCOUNT GETUSEDLICENSESPERFEATUREINDEXCOUNT
  #define F90_GETBORROWEDFEATURESWITHVERSION GETBORROWEDFEATURESWITHVERSION
  #define F90_CHECKOUTWITHORIGIN CHECKOUTWITHORIGIN
#endif

/**
 * Initialize License Library Client with default settings.
 */
bool initialize();

/**
 * Initialize License Library Client.
 * @param showDialogs enables the showing of error dialogs.
 * @param heartbeat enable or disable the heartbeat mechanism.
 * @return success or fail
 */
bool initialize(bool showDialogs, bool heartbeat);

/**
 * Initialize License Library Client.
 * @param showDialogs enables the showing of error dialogs.
 * @param heartbeat enable or disable the heartbeat mechanism.
 * @param path overwrites default license search path
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_INITIALIZE(bool showDialogs, bool heartbeat, const char * path, unsigned int len_path); }
bool initialize(bool showDialogs, bool heartbeat, const char * path);

/**
* Turn on borrowing, set expiration date and time.
* @param date time for expiration dd-mmm-yyyy(:[HH:MM]), time is optional. Example: 01-dec-2013:15:48
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_ENABLEBORROW(const char * datetime, unsigned int len_datetime); }
bool enableBorrowing(const char * datetime);

/**
* Turn off borrowing.
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_DISABLEBORROW(); }
bool disableBorrowing();

/**
* Get the maximum timespan in minutes that is left for a borrowed feature (rounded up).
* @param feature to request.
* @param output is the address of the char[] to write the result to.
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_BORROWTIMELIMIT(const char * feature, int & minutes, unsigned int len_feature); }
bool getBorrowingTimeLimit(const char * feature, int & minutes);

/**
* Get the maximum borrow time in hours for a borrowed feature (rounded up).
* @param feature to request.
* @param output is the address of the char[] to write the result to.
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_MAXBORROWTIME(const char * feature, int & hours, unsigned int len_feature); }
bool getMaxBorrowTime(const char * feature, int & hours);


/**
* Return borrowed feature early.
* @param feature to return.
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_RETURNBORROWEDFEATURE(const char * feature, unsigned int len_feature); }
bool returnBorrowedFeature(const char * feature);

/**
* List borrowed features.
* @param output is the address of the char[] to write the result to.
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_GETBORROWEDFEATURES(char * f90_result, unsigned int len_f90_result); }
bool getBorrowedFeatures(char ** output);

/**
 * Checkout a license of a given feature.
 * @param feature to checkout.
 * @version of the feature to checkout.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_CHECKOUT(const char * feature, const char * version, unsigned int len_feature, unsigned int len_version); }
bool  checkout(const char * feature, const char * version);

/**
 * Return a license of a given feature.
 * @param feature to checkin.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_CHECKIN(const char * feature, unsigned int len_feature); }
bool checkin(const char * feature);

/**
 * Return vendor of a given feature.
 * @param feature to query.
 * @param result is the address of the char[] to write the result to.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_GETVENDOR(const char * feature, char * f90_result, unsigned int len_feature, unsigned int len_f90_result); }
bool getVendor(const char * feature, char ** result);

/**
 * Return distributor of a given feature.
 * @param feature to query.
 * @param result is the address of the char[] to write the result to.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_GETDISTRIBUTOR(const char * feature, char * f90_result, unsigned int len_feature, unsigned int len_f90_result); }
bool getDistributor(const char * feature, char ** result);

/**
 * Return notice of a given feature.
 * @param feature to query.
 * @param result is the address of the char[] to write the result to.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_GETNOTICE(const char * feature, char * f90_result, unsigned int len_feature, unsigned int len_f90_result); }
bool getNotice(const char * feature, char ** result);

/**
 * Return username of a given feature.
 * @param feature to query.
 * @param is the address of the char[] to write the result to.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_GETUSERNAME(const char * feature, char * f90_result, unsigned int len_feature, unsigned int len_f90_result); }
bool getUsername(const char * feature, char ** result);

/**
 * Return user count of a given feature (as string).
 * @param feature to query.
 * @param is the address of the int to write the result to.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_GETCOUNT(const char * feature, int & result, unsigned int len_feature); }
bool getCount(const char * feature, int & result);

/**
 * Return expiration date of a given feature (as string).
 * @param feature to query.
 * @param is the address of the char[] to write the result to.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_GETEXPIRATION(const char * feature, char * f90_result, unsigned int len_feature, unsigned int len_f90_result); }
bool getExpiration(const char * feature, char ** result);

/**
 * Return current users of a given feature (as string).
 * @param feature to query.
 * @param is the address of the char[] to write the result to.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_GETCURRENTUSERS(const char * feature, char * f90_result, unsigned int len_feature, unsigned int len_f90_result); }
bool getCurrentUsers(const char * feature, char ** result);

/**
 * Return licenses used by a user of a given feature (as string).
 * @param feature to query.
 * @param username.
 * @param is the address of the int to write the result to.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_GETUSEDLICENSES(const char * feature, char * user, int & result, unsigned int len_feature, unsigned int len_user); }
bool getUsedLicenses(const char * feature, char * user, int & result);

/**
 * Return available licenses of a given feature (as string).
 * @param feature to query.
 * @param is the address of the int to write the result to.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_GETAVAILABLELICENSES(const char * feature, int & result, unsigned int len_feature); }
bool getAvailableLicenses(const char * feature, int & result);

/**
 * Return all available features (as string).
 * @param is the address of the char[] to write the result to.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_GETAVAILABLEFEATURES(char * result, unsigned int len_f90_result); }
bool getAvailableFeatures(char ** result);

/**
* Test if the license server is available
* @param result contains the names of the not-available servers on failure, lists available servers on success.
* @return success or fail
*/
extern "C" { bool FTN_CALL F90_ISSERVERAVAILABLE(char * result, unsigned int len_f90_result); }
bool isServerAvailable(char ** result);

/**
 * Return if a given feature is from the server (as string).
 * @param feature to query.
 * @param server yes/no.
 * @return success or fail
 */
extern "C" { bool FTN_CALL F90_ISSERVER(const char * feature, bool & result, unsigned int len_feature); }
bool isServer(const char * feature, bool & result);

/**
 * Get the error message stack generated by the Licensing Library.
 * @return error lines as char[] with carrage return as splitter
 */
extern "C" { char * FTN_CALL F90_GETERRORS(char * f90_result, unsigned int len_f90_result); }
const char * getErrors();

/**
 * Get the last error message generated by the Licensing Library.
 * @return last error line as char[]
 */
extern "C" { char *  FTN_CALL F90_GETLASTERROR(char * f90_result, unsigned int len_f90_result); }
const char * getLastError();

/**
 * Enable or disable the dialogs mechanism (needed for VB6, no constructor overloading).
 * @param boolean.
 * @return success or fail
 */
bool setDialogs(bool dialogs_on);

/**
 * Enable or disable the heartbeat mechanism (needed for VB6, no constructor overloading).
 * @param boolean.
 * @return success or fail
 */
bool setHeartbeat(bool heartbeat_on);

/**
* Return the version of a given feature.
* @param feature to query.
* @param result is the address of the char[] to write the result to.
* @return success or fail
*/
extern "C" { bool FTN_CALL F90_GETFEATUREVERSION(const char * feature, char * f90_result, unsigned int len_feature, unsigned int len_f90_result); }
bool getFeatureVersion(const char * feature, char ** result);

/**
 * Free library and cleanup resources
 */
extern "C" { void FTN_CALL F90_CLEANUP(); }
void cleanup();

/**
* Get the number of licenses for a specific feature.
* @param feature to request.
* @param output is the number of licenses for the feature.
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_GETNUMBEROFLICENSESPERFEATURE(const char * feature, int & f90_result); }
bool getNumberOfLicensesPerFeature(const char * feature, int & f90_result);

/**
* Get is server or local at given feature index.
* @param feature to request.
* @param index is the feature index.
* @param output is the ref with the result: 1 server, 2 local.
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_GETISSERVERPERFEATUREINDEX(const char * feature, const int & index, int & f90_result); }
bool getLicenseTypePerFeatureIndex(const char * feature, const int & index, int & f90_result);

/**
* Get the version at at given feature index.
* @param feature to request.
* @param index is the feature index.
* @param output is the ref to fill with the version.
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_GETFEATUREVERSIONPERFEATUREINDEX(const char * feature, const int & index, char * f90_result); }
bool getFeatureVersionPerFeatureIndex(const char * feature, const int & index, char ** f90_result);

/**
* Get the version at at given feature index.
* @param feature to request.
* @param index is the feature index.
* @param result is the number of available licenses at given index
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_GETAVAILABLELICENSESPERFEATUREINDEXCOUNT(const char * feature, const int & index, int & f90_result); }
bool getAvailableLicensesPerFeatureIndexCount(const char * feature, const int & index, int & f90_result);

/**
* Get used licenses at given feature index.
@param feature to request.
* @param index is the feature index.
* @param result is the number of used licenses at given index
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_GETUSEDLICENSESPERFEATUREINDEXCOUNT(const char * feature, const int & index, int & f90_result); }
bool getUsedLicensesPerFeatureIndexCount(const char * feature, const int & index, int & f90_result);

/**
* List borrowed features with the version number appended.
* @param output is the address of the char[] to write the result to.
* @return success or failure
*/
extern "C" { bool FTN_CALL F90_GETBORROWEDFEATURES(char * f90_result, unsigned int len_f90_result); }
bool getBorrowedFeaturesWithVersion(char ** output);

/**
* Checkout a license of a given feature, with origin information.
* @param feature to checkout.
* @version of the feature to checkout.
* @isServer 1 = checkout from server license, 0 = checkout from local license.
* @return success or fail
*/
extern "C" { bool FTN_CALL F90_CHECKOUTWITHORIGIN(const char * feature, const char * version, unsigned int len_feature, unsigned int len_version, int & isServer); }
bool checkoutLicenseWithOriginInformation(const char * feature, const char * version, int & isServer);

#endif
