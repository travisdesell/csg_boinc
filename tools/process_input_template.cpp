// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "filesys.h"
#include "md5_file.h"
#include "str_replace.h"

#include "sched_config.h"
#include "sched_util.h"

#include "process_input_template.h"

using std::string;
using std::vector;

// look for file named FILENAME.md5 containing md5sum and length.
// If found, and newer mod time than file,
// read md5 sum and file length from it.
// See checkin notes Dec 30 2004
//
static bool got_md5_info(
    const char *path,
    char *md5data,
    double *nbytes
) {
    char md5name[512];
    struct stat md5stat, filestat;
    char endline='\0';

    sprintf(md5name, "%s.md5", path);
    
    // get mod times for file
    //
    if (stat(path, &filestat)) {
        return false;
    }
  
    // get mod time for md5 cache file
    //
    if (stat(md5name, &md5stat)) {
        return false;
    }
    
    // if cached md5 newer, then open it
#ifndef _USING_FCGI_
    FILE *fp=fopen(md5name, "r");
#else
    FCGI_FILE *fp=FCGI::fopen(md5name, "r");
#endif
    if (!fp) {
        return false;
    }
  
    // read two quantities: md5 sum and length.
    // If we can't read these, or there is MORE stuff in the file,
    // it's not an md5 cache file
    //
    int n = fscanf(fp, "%s %lf%c", md5data, nbytes, &endline);
    int c = fgetc(fp);
    fclose(fp);
    if ((n != 3) || (endline !='\n') || (c != EOF)) {
        fprintf(stderr, "bad MD5 cache file %s; remove it and stage file again\n", md5name);
        return false;
    }

    // if this is one of our cached md5 files, but it's OLDER than the
    // data file which it supposedly corresponds to, delete it.
    //
    if (md5stat.st_mtime<filestat.st_mtime) {
        unlink(md5name);
        return false;
    }
    return true;
}

// Write file FILENAME.md5 containing md5sum and length
// see checkin notes Dec 30 2004
//
static void write_md5_info(
    const char *path,
    const char *md5,
    double nbytes
) {
    char md5name[512];
    struct stat statbuf;
    int retval;
    
    // if file already exists with this name, don't touch it.
    //
    sprintf(md5name, "%s.md5", path);
    if (!stat(md5name, &statbuf)) {
        return;
    }

#ifndef _USING_FCGI_
    FILE *fp=fopen(md5name, "w");
#else
    FCGI_FILE *fp=FCGI::fopen(md5name, "w");
#endif

    // if can't open the file, give up
    //
    if (!fp) {
        return;
    }
  
    retval = fprintf(fp,"%s %.15e\n", md5, nbytes);
    fclose(fp);

    // if we didn't write properly to the file, delete it.
    //
    if (retval<0) {
        unlink(md5name);
    }
  
    return;
}

static bool input_file_found[1024];

// generate a <file_info> element for workunit XML doc,
// based on the input template and list of files
//
// Inputs:
// xp: parser for input template
// ninfiles, infiles: list of physical filenames
//
// Output:
// out: the <file_info> element for the WU XML doc

static int process_file_info(
    XML_PARSER& xp, string& out, vector<INFILE_DESC> infiles,
    SCHED_CONFIG& config_loc
) {
    vector<string> urls;
    bool gzip = false;
    int retval, file_number = -1;
    double nbytes, nbytesdef = -1, gzipped_nbytes;
    string md5str, urlstr, tmpstr;
    char buf[BLOB_SIZE], path[MAXPATHLEN], top_download_path[MAXPATHLEN];
    char gzip_path[MAXPATHLEN];
    char md5[33], url[256], gzipped_url[256], buf2[256];

    out += "<file_info>\n";
    while (!xp.get_tag()) {
        if (xp.parse_int("number", file_number)) {
            continue;
        } else if (xp.parse_bool("gzip", gzip)) {
            continue;
        } else if (xp.parse_string("url", urlstr)) {
            urls.push_back(urlstr);
            continue;
        } else if (xp.parse_string("md5_cksum", md5str)) {
            continue;
        } else if (xp.parse_double("nbytes", nbytesdef)) {
            continue;
        } else if (xp.parse_double("gzipped_nbytes", gzipped_nbytes)) {
            continue;
        } else if (xp.match_tag("/file_info")) {
            if (nbytesdef != -1 || md5str != "" || urlstr != "") {
                if (nbytesdef == -1 || md5str == "" || urlstr == "") {
                    fprintf(stderr, "All file properties must be defined "
                        "if at least one is defined (url, md5_cksum, nbytes)!\n"
                    );
                    return ERR_XML_PARSE;
                }
                if (gzip && !gzipped_nbytes) {
                    fprintf(stderr, "Must specify gzipped_nbytes\n");
                    return ERR_XML_PARSE;
                }
            }
            if (file_number < 0) {
                fprintf(stderr, "No file number found\n");
                return ERR_XML_PARSE;
            }
            if (file_number >= (int)infiles.size()) {
                fprintf(stderr,
                    "Too few input files given; need at least %d\n",
                    file_number+1
                );
                return ERR_XML_PARSE;
            }
            if (input_file_found[file_number]) {
                fprintf(stderr,
                    "Input file %d listed twice\n", file_number
                );
                return ERR_XML_PARSE;
            }
            input_file_found[file_number] = true;
            INFILE_DESC& infile = infiles[file_number];

            if (nbytesdef > 0) {
                // here if the file was specified in the input template;
                // i.e it's already staged, possibly remotely
                //
                urlstr = "";
                for (unsigned int i=0; i<urls.size(); i++) {
                    urlstr += "    <url>" + urls.at(i) + string(infile.name) + "</url>\n";
                    if (gzip) {
                        urlstr += "    <gzipped_url>" + urls.at(i) + string(infile.name) + ".gz</gzipped_url>\n";
                    }
                }
                sprintf(buf,
                    "    <name>%s</name>\n"
                    "%s"
                    "    <md5_cksum>%s</md5_cksum>\n"
                    "    <nbytes>%.0f</nbytes>\n",
                    infile.name,
                    urlstr.c_str(),
                    md5str.c_str(),
                    nbytesdef
                );
                if (gzip) {
                    sprintf(buf2, "    <gzipped_nbytes>%.0f</gzipped_nbytes>\n",
                        gzipped_nbytes
                    );
                    strcat(buf, buf2);
                }
                strcat(buf, "</file_info>\n");
            } else if (infile.is_remote) {
                sprintf(buf,
                    "    <name>jf_%s</name>\n"
                    "    <url>%s</url>\n"
                    "    <md5_cksum>%s</md5_cksum>\n"
                    "    <nbytes>%.0f</nbytes>\n"
                    "</file_info>\n",
                    infile.md5,
                    infile.url,
                    infile.md5,
                    infile.nbytes
                );
            } else {
                // here if file is local; we need to find its size and MD5;
                // stage the file if needed
                //
                dir_hier_path(
                    infile.name, config_loc.download_dir,
                    config_loc.uldl_dir_fanout, path, true
                );

                // if file isn't found in hierarchy,
                // look for it at top level and copy
                //
                if (!boinc_file_exists(path)) {
                    sprintf(top_download_path,
                        "%s/%s",config_loc.download_dir, infile.name
                    );
                    boinc_copy(top_download_path, path);
                }

                if (!config_loc.cache_md5_info || !got_md5_info(path, md5, &nbytes)) {
                    retval = md5_file(path, md5, nbytes);
                    if (retval) {
                        fprintf(stderr,
                            "process_input_template: md5_file %s\n",
                            boincerror(retval)
                        );
                        return retval;
                    } else if (config_loc.cache_md5_info) {
                        write_md5_info(path, md5, nbytes);
                    }
                }

                dir_hier_url(
                    infile.name, config_loc.download_url,
                    config_loc.uldl_dir_fanout, url
                );

                if (gzip) {
                    sprintf(gzip_path, "%s.gz", path);
                    retval = file_size(gzip_path, gzipped_nbytes);
                    if (retval) {
                        fprintf(stderr,
                            "process_input_template: missing gzip file %s\n",
                            gzip_path
                        );
                        return ERR_FILE_MISSING;
                    }
                    sprintf(gzipped_url,
                        "    <gzipped_url>%s.gz</gzipped_url>\n"
                        "    <gzipped_nbytes>%.0f</gzipped_nbytes>\n",
                        url, gzipped_nbytes
                    );
                } else {
                    strcpy(gzipped_url, "");
                }

                sprintf(buf,
                    "    <name>%s</name>\n"
                    "    <url>%s</url>\n"
                    "%s"
                    "    <md5_cksum>%s</md5_cksum>\n"
                    "    <nbytes>%.0f</nbytes>\n"
                    "</file_info>\n",
                    infile.name,
                    url,
                    gzipped_url,
                    md5,
                    nbytes
                );
            }
            out += buf;
            break;
        } else {
            // copy any other elements from input template to XML doc
            //
            retval = xp.copy_element(tmpstr);
            if (retval) return retval;
            out += tmpstr;
            out += "\n";
        }
    }
    return 0;
}

static int process_workunit(
    XML_PARSER& xp, WORKUNIT& wu, string& out,
    vector<INFILE_DESC> &infiles,
    const char* command_line,
    const char* additional_xml
) {
    char buf[256], open_name[256];
    int file_number;
    string tmpstr, cmdline;
    int retval;

    out += "<workunit>\n";
    if (command_line) {
        //fprintf(stderr, "appending command line: %s\n", command_line);
        out += "<command_line>\n";
        out += command_line;
        out += "\n</command_line>\n";
    }
    while (!xp.get_tag()) {
        if (xp.match_tag("/workunit")) {
            if (additional_xml && strlen(additional_xml)) {
                out += additional_xml;
                out += "\n";
            }
            out += "</workunit>";
            break;
        } else if (xp.match_tag("file_ref")) {
            out += "<file_ref>\n";
            bool found_file_number = false, found_open_name = false;
            while (!xp.get_tag()) {
                if (xp.parse_int("file_number", file_number)) {
                    INFILE_DESC& id = infiles[file_number];
                    if (id.is_remote) {
                        sprintf(buf, "    <file_name>jf_%s</file_name>\n",
                            infiles[file_number].md5
                        );
                    } else {
                        sprintf(buf, "    <file_name>%s</file_name>\n",
                            infiles[file_number].name
                        );
                    }
                    out += buf;
                    found_file_number = true;
                    continue;
                } else if (xp.parse_str("open_name", open_name, sizeof(open_name))) {
                    sprintf(buf, "    <open_name>%s</open_name>\n", open_name);
                    out += buf;
                    found_open_name = true;
                    continue;
                } else if (xp.match_tag("/file_ref")) {
                    if (!found_file_number) {
                        fprintf(stderr, "No file number found\n");
                        return ERR_XML_PARSE;
                    }
                    if (!found_open_name) {
                        fprintf(stderr, "No open name found\n");
                        return ERR_XML_PARSE;
                    }
                    out += "</file_ref>\n";
                    break;
                } else if (xp.parse_string("file_name", tmpstr)) {
                    fprintf(stderr, "<file_name> ignored in <file_ref> element.\n");
                    continue;
                } else {
                    retval = xp.copy_element(tmpstr);
                    if (retval) return retval;
                    out += tmpstr;
                    out += "\n";
                }
            }
        } else if (xp.parse_string("command_line", cmdline)) {
            if (command_line) {
                fprintf(stderr, "Can't specify command line twice\n");
                return ERR_XML_PARSE;
            }
            out += "<command_line>\n";
            out += cmdline;
            out += "\n</command_line>\n";
        } else if (xp.parse_double("rsc_fpops_est", wu.rsc_fpops_est)) {
            continue;
        } else if (xp.parse_double("rsc_fpops_bound", wu.rsc_fpops_bound)) {
            continue;
        } else if (xp.parse_double("rsc_memory_bound", wu.rsc_memory_bound)) {
            continue;
        } else if (xp.parse_double("rsc_bandwidth_bound", wu.rsc_bandwidth_bound)) {
            continue;
        } else if (xp.parse_double("rsc_disk_bound", wu.rsc_disk_bound)) {
            continue;
        } else if (xp.parse_int("batch", wu.batch)) {
            continue;
        } else if (xp.parse_int("delay_bound", wu.delay_bound)){
            continue;
        } else if (xp.parse_int("min_quorum", wu.min_quorum)) {
            continue;
        } else if (xp.parse_int("target_nresults", wu.target_nresults)) {
            continue;
        } else if (xp.parse_int("max_error_results", wu.max_error_results)) {
            continue;
        } else if (xp.parse_int("max_total_results", wu.max_total_results)) {
            continue;
        } else if (xp.parse_int("max_success_results", wu.max_success_results)) {
            continue;
        } else if (xp.parse_int("size_class", wu.size_class)) {
            continue;
        } else {
            retval = xp.copy_element(tmpstr);
            if (retval) return retval;
            out += tmpstr;
            out += "\n";
        }
    }
    return 0;
}

// fill in the workunit's XML document (wu.xml_doc)
// by scanning the input template, macro-substituting the input files,
// and putting in the command line element and additional XML
//
int process_input_template(
    WORKUNIT& wu,
    char* tmplate,
    vector<INFILE_DESC> &infiles,
    SCHED_CONFIG& config_loc,
    const char* command_line,
    const char* additional_xml
) {
    string out;
    int retval;
    bool found_workunit=false;
    int nfiles_parsed = 0;

    for (int i=0; i<1024; i++) {
        input_file_found[i] = false;
    }

    out = "";
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_buf_read(tmplate);
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.match_tag("input_template")) continue;
        if (xp.match_tag("/input_template")) continue;
        if (xp.match_tag("file_info")) {
            retval = process_file_info(xp, out, infiles, config_loc);
            if (retval) return retval;
            nfiles_parsed++;
        } else if (xp.match_tag("workunit")) {
            found_workunit = true;
            retval = process_workunit(
                xp, wu, out, infiles, command_line, additional_xml
            );
            if (retval) return retval;
        }
    }
    if (!found_workunit) {
        fprintf(stderr, "process_input_template: bad WU template - no <workunit>\n");
        return ERR_XML_PARSE;
    }
    if (nfiles_parsed != (int)infiles.size()) {
        fprintf(stderr,
            "process_input_template: %d input files listed, but template has %d\n",
            (int)infiles.size(), nfiles_parsed
        );
        return ERR_XML_PARSE;
    }
    if (out.size() > sizeof(wu.xml_doc)-1) {
        fprintf(stderr,
            "create_work: WU XML field is too long (%d bytes; max is %d)\n",
            (int)out.size(), (int)sizeof(wu.xml_doc)-1
        );
        return ERR_BUFFER_OVERFLOW;
    }
    //fprintf(stderr, "copying to xml_doc: %s\n", out.c_str());
    safe_strcpy(wu.xml_doc, out.c_str());
    return 0;
}
