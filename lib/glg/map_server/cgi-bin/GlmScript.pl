#!C:\perl\bin\perl.exe

##########################################################################
# To use this script, follow the instructions below.
# 
# 1. Install perl. A free implementation of perl for Windows can be 
#    downloaded at www.activeperl.com. Perl must be installed after the
#    web server so that application bindings are properly installed. 
#    Refer to the Web Server Installation Notes Appendix of the GLG Map Server
#    Reference Manual for more information on installing perl. 
#
# 2. IIS ON WINDOWS ONLY: Turn on IIS by checking the Internet Information 
#    Services box in the Windows Features dialog. Refer to the following link
#    for details:
#       http://www.betterhostreview.com/turn-on-iis-windows-10.html
#
#    In the Windows Features dialog described in the above link,
#    check the box for the CGI Windows feature located at:
#       Internet Information Service->World Wide Web services->
#          Application Development Features->CGI
#
#    Create Scripts directory in the IIS root directory 
#    (C:\inetpur\wwwroot\Script) if it doesn't exist and add a
#    script mapping for this directory to map "*.pl" to the perl executable.
#    Refer to the following link for an example:
#       http://gdavidcarter.blogspot.com/2016/04/installing-perl-on-windows-2012r2-iis-85.html
#
# 3. Copy the following files to the web server's cgi-bin scripts directory
#    (cgi-bin directory for Apache or Scripts directory for IIS):
#      - the map server executable <glg_dir>/bin/GlmMap (GlmMap.exe on Windows)
#      - the perl script <glg_dir>/map_server/cgi-bin/GlgScript.pl
#      - the dataset directory <glg_dir>/map_data 
#      - the error log file <glg_dir>/map_server/cgi-bin/glm_err.log
#
# 4. LINUX/UNIX ONLY: Make sure GlmMap and GlmScript have execute permission 
#    (refer to the Map Server INSTALLATION INSTRUCTIONS for details).
#
# 5. Change permissions of the glm_err.log file to make it writable
#    by the cgi-bin executables. This file will contain the map server's 
#    error messages. 
#    ON LINUX/UNIX, make the glm_err.log file in the cgi-bin directory
#    writable by the web server's cgi-bin (www-data user and/or group in the 
#    default linux/apache configuration). 
#    FOR IIS ON WINDOWS, change security permissions of the glm_err.log file 
#    in the Scripts directory to give full access to all users.
#
# 6. Edit the MAP_SERVER_EXE variable in the script below to point to the
#    location of the map server executable.
#
# 7. Edit the DATASET variable in the script below to point to the 
#    location of the dataset's sdf file. 
#
# 8. If an evaluation version of the map server is used, set the 
#    GLG_EVAL_STRING variable in the script below to the up-to-date
#    evaluation code obtained from Generic Logic (www.genlogic.com).
#    For the production version of the map server, comment out 
#    GLG_EVAL_STRING settings.
#
# 9. To debug the map server setup, uncomment lines for extra logging and 
#    verbosity in the script below. The diagnostic output will be written
#    into the glm_err.log file.
#
# The following sample requests use the default demo dataset and may be 
# used to test a map server installation. To use them, cut and paste a 
# request into the location field of the browser, replacing "my_web_server"
# with the hostname of the web server where the map server is installed. 
# Replace "Scripts" with a valid path to the GlmScript.pl.
#
#http://my_web_server.com/Scripts/GlmScript.pl?VERSION=1.3.0&REQUEST=GetMap&SRS=AUTO2:42003,1.,10,40&WIDTH=714&HEIGHT=512&BBOX=-5102508.8,-5085401.6,5102508.8,5085401.6&BGCOLOR=0x0&STYLES=default&FORMAT=image/jpeg&LAYERS=default&STRETCH=0
#
#http://my_web_server.com/Scripts/GlmScript.pl?VERSION=1.3.0&REQUEST=GetMap&SRS=AUTO2:42003,1.,10,40&WIDTH=714&HEIGHT=512&BBOX=-5102508.8,-5085401.6,5102508.8,5085401.6&BGCOLOR=0x0&STYLES=default&FORMAT=image/jpeg&LAYERS=earth,political,grid&STRETCH=0
#http://my_web_server.com/Scripts/GlmScript.pl?VERSION=1.3.0&REQUEST=GetMap&SRS=EPSG:4326&WIDTH=800&HEIGHT=700&BBOX=-180,-90,180,90&BGCOLOR=0x0&STYLES=default&FORMAT=image/jpeg&LAYERS=earth&STRETCH=0
#
# To test the map server in the stand-alone mode, use the request_* scripts
# (request_*.bat scripts on Windows) from the map_data directory. 
# Edit the scripts first to provide correct paths.
##########################################################################

##########################################################################
# Configuration variables for Unix/Linux and Windows
#
# This example assumes that the map server executable and the
# map_data directory are located in the web server's cgi-bin directory.
##########################################################################

# Uncomment the following for Unix/Linux
#$MAP_SERVER_EXE="./GlmMap";
#$DATASET="./map_data/sample.sdf";
#$ERROR_LOG=">> ./glm_err.log";

# Uncomment the following for IIS on Windows. For Apache on Windows, change
# the path to the cgi-bin directory of the Apache web server.
#$CGI_DIR="C:/inetpub/wwwroot/Scripts";
#$MAP_SERVER_EXE="${CGI_DIR}/GlmMap.exe";
#$DATASET="${CGI_DIR}/map_data/sample.sdf";
#$ERROR_LOG=">> ${CGI_DIR}/glm_err.log";

##########################################################################
# Uncomment the following lines for extra logging
#
#open(ERRFILE, $ERROR_LOG);
#$TIME_STRING = localtime(time());
#print ERRFILE "\n";
#print ERRFILE "NEW MAP REQUEST AT DATE: ";
#print ERRFILE $TIME_STRING;
#print ERRFILE "\n";
#print ERRFILE "QUERY_STRING=$ENV{'QUERY_STRING'}\n";
#close(ERRFILE);

##########################################################################
# Uncomment the following line to generate map server debugging 
# information
#
#$VERBOSITY="-verbosity 1";

##########################################################################
# The following string is required to use the evaluation version of the 
# map server. Put a most recent eval string here. Comment out 
# GLG_EVAL_STRING setting for the production or free non-commercial 
# version of the map server.
##########################################################################
#$ENV{GLG_EVAL_STRING}="PLACE_A_VALID_GLG_EVAL_STRING_HERE";

##########################################################################
# Use the following line to test map server installation
##########################################################################
system "$MAP_SERVER_EXE -generate -cgi $VERBOSITY -dataset $DATASET 2$ERROR_LOG";

##########################################################################
# Use the following line for production installation
##########################################################################
#system "$MAP_SERVER_EXE -generate -cgi $VERBOSITY -dataset $DATASET";
