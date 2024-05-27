#include <array>
#include <filesystem>
#include <iomanip>
#include <nefis_exception.h>
#include <nefis_file.h>
#include <sstream>
#include <string>
#include <vector>

extern "C"
{
#include <nefis.h>
}

void nefis_file::set_file(std::string const &filein)
{
    file_name = filein;
}

int nefis_file::open()
{
    if (_read_only)
        return this->open('r');
    else
        return this->open('u');
}

int nefis_file::open(char access_modifier)
{
    file_status_open = false;
    if (!(access_modifier == 'c' || access_modifier == 'r' || access_modifier == 'u'))
        throw new std::exception("Invalid access modified argument");
    if (std::filesystem::exists(file_name))
    {
        char coding = ' ';
        auto filename_copy = std::make_unique<char[]>(file_name.length() + 1);
        file_name.copy(filename_copy.get(), file_name.length() + 1);
        int retval = Crenef(&file_pointer, filename_copy.get(), filename_copy.get(), coding, access_modifier);
        if (retval != 0)
        {
            throw nefis_exception(this);
        }
        file_status_open = true;
        return 0;
    }
    throw nefis_exception("Error: " + file_name + " does not exist");
}

int nefis_file::close()
{
    int retval = Clsnef(&file_pointer);
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    file_status_open = false;
    return retval;
}

void nefis_file::write_float_elements(std::string groupname, std::string elementname, nefis_uindex uindex,
                                      std::vector<float> buffer)
{
    std::array uindex_ = {uindex.start, uindex.end, uindex.step};
    std::array usrord = std_order;
    auto groupname_ = std::make_unique<char[]>(groupname.length() + 1);
    groupname.copy(groupname_.get(), groupname.length() + 1);
    auto elementname_ = std::make_unique<char[]>(elementname.length() + 1);
    elementname.copy(elementname_.get(), elementname.length() + 1);

    auto retval =
        Putelt(&file_pointer, groupname_.get(), elementname_.get(), uindex_.data(), usrord.data(), buffer.data());
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
}

void nefis_file::write_float_elements(std::string groupname, std::string elementname, nefis_uindex uindex_1st_dim,
                                      nefis_uindex uindex_2nd_dim, std::vector<std::vector<float>> buffer)
{
    std::array uindex = {uindex_1st_dim.start, uindex_1st_dim.end, uindex_1st_dim.step,
                         uindex_2nd_dim.start, uindex_2nd_dim.end, uindex_2nd_dim.step};
    std::array usrord = std_order;

    auto elementname_ = std::make_unique<char[]>(elementname.length() + 1);
    elementname.copy(elementname_.get(), elementname.length() + 1);
    auto groupname_ = std::make_unique<char[]>(groupname.length() + 1);
    groupname.copy(groupname_.get(), groupname.length() + 1);

    // regrouping the data
    std::vector<float> alldata;
    int el_size = get_element_dimension(elementname);
    for (std::vector<float> vec_dat : buffer)
    {
        // check if the data fits in the length
        alldata.insert(alldata.end(), vec_dat.begin(), vec_dat.end());
    }
    auto retval =
        Putelt(&file_pointer, groupname_.get(), elementname_.get(), uindex.data(), usrord.data(), alldata.data());
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
}

void nefis_file::write_int_elements(std::string groupname, std::string elementname, nefis_uindex uindex,
                                    std::vector<int> buffer)
{
    std::array uindex_ = {uindex.start, uindex.end, uindex.step};
    std::array usrord = std_order;
    auto groupname_ = std::make_unique<char[]>(groupname.length() + 1);
    groupname.copy(groupname_.get(), groupname.length() + 1);
    auto elementname_ = std::make_unique<char[]>(elementname.length() + 1);
    elementname.copy(elementname_.get(), elementname.length() + 1);

    int *buf_pt = buffer.data();
    auto retval = Putelt(&file_pointer, groupname_.get(), elementname_.get(), uindex_.data(), usrord.data(), buf_pt);
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
}

void nefis_file::write_string_elements(const std::string &groupname, const std::string &elementname,
                                       nefis_uindex uindex, int stringlength, std::vector<std::string> buffer)
{
    size_t size = buffer.size();
    std::array uindex_ = {uindex.start, uindex.end, uindex.step};
    std::array usrord = std_order;
    auto groupname_ = std::make_unique<char[]>(groupname.length() + 1);
    groupname.copy(groupname_.get(), groupname.length() + 1);
    auto elementname_ = std::make_unique<char[]>(elementname.length() + 1);
    elementname.copy(elementname_.get(), elementname.length() + 1);
    int stringlength2 = stringlength;

    if (stringlength2 == 0)
    {
        stringlength2 = get_string_length(elementname);
    }
    auto buflen = size * stringlength2 + 1; // add 1 byte for \0
    auto pt = std::make_unique<char[]>(buflen);

    std::stringstream stringstream;
    std::for_each(buffer.begin(), buffer.end(),
                  [&](std::string &str) { stringstream << std::setw(stringlength2) << std::left << str; });
    auto data = stringstream.str();
    if (data.length() > buflen)
    {
        throw std::runtime_error("write_string_elements(): Incorrect buffer size! writing to group " + groupname +
                                 " and element " + elementname + " size is " + std::to_string(data.length()) +
                                 " should be " + std::to_string(buflen) + " size " + std::to_string(size) +
                                " String length " + std::to_string(stringlength));
    }
    data.copy(pt.get(), data.length());
    auto retval = Putelt(&file_pointer, groupname_.get(), elementname_.get(), uindex_.data(), usrord.data(), pt.get());
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
}

int nefis_file::get_string_length(std::string_view element_name)
{
    // fixed length arrays based on NEFIS5 spec.
    char elmtyp[9];
    char elmquant[17];
    char elmun[17];
    char elmdes[65];
    int size_string;
    int elmdm = 5;
    int elmdms[5];

    auto elementname_ = std::make_unique<char[]>(element_name.length() + 1);
    element_name.copy(elementname_.get(), element_name.length() + 1);
    auto retval = Inqelm(&file_pointer, elementname_.get(), elmtyp, &size_string, elmquant, elmun, elmdes, &elmdm, elmdms);

    if( retval != 0)
    {
        throw nefis_exception(this);
    }
    return size_string;
}

void nefis_file::write_string_elements(const std::string &groupname, const std::string &elementname,
                                       nefis_uindex uindex_1st_dim, nefis_uindex uindex_2nd_dim, int stringlength,
                                       const std::vector<std::vector<std::string>> &buffer)
{
    size_t size = buffer.capacity();
    size_t size2 = buffer[0].capacity();
    std::array uindex = {uindex_1st_dim.start, uindex_1st_dim.end, uindex_1st_dim.step,
                         uindex_2nd_dim.start, uindex_2nd_dim.end, uindex_2nd_dim.step};
    std::array usrord = std_order;



    const auto groupname_ = std::make_unique<char[]>(groupname.length() + 1);
    groupname.copy(groupname_.get(), groupname.length() + 1);
    auto elementname_ = std::make_unique<char[]>(elementname.length() + 1);
    elementname.copy(elementname_.get(), elementname.length() + 1);
    int stringlength2 = stringlength;

    if (stringlength2 == 0)
    {
        stringlength2 = get_string_length(elementname);
    }

    const auto buflen = size2 * size * stringlength2 + 1;
    const auto pt = std::make_unique<char[]>(buflen);

    std::stringstream stringstream;
    for (auto &vecstr : buffer)
    {
        std::for_each(vecstr.begin(), vecstr.end(),
                      [&](std::string str) { stringstream << std::setw(stringlength2) << std::left << str; });
    }
    const auto data = stringstream.str();
    if (data.length() > buflen)
    {
        throw std::runtime_error("write_string_elements(): Incorrect buffer size! writing to group " + groupname +
                                 " and element " + elementname + " size is " + std::to_string(data.length()) +
                                 " should be " + std::to_string(buflen) + " size " + std::to_string(size) + " size2 " +
                                 std::to_string(size2) + " String length " + std::to_string(stringlength));
    }
    data.copy(pt.get(), data.length());

    auto retval = Putelt(&file_pointer, groupname_.get(), elementname_.get(), uindex.data(), usrord.data(), pt.get());
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
}

int nefis_file::get_group_dim(std::string grpname) const
{
    char celnam[16 + 1];
    int grpndm = 1;
    int grpdms[5];
    int grpord[5];

    auto groupname2 = std::make_unique<char[]>(grpname.length() + 1);
    grpname.copy(groupname2.get(), grpname.length());
    auto retval = Inqgrp(&file_pointer, groupname2.get(), celnam, &grpndm, grpdms, grpord);
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    return grpndm;
}

int nefis_file::get_int_attribute(const std::string &groupname, const std::string &attributename) const
{
    auto groupname2 = std::make_unique<char[]>(groupname.length() + 1);
    groupname.copy(groupname2.get(), groupname.length() + 1);
    auto attnam2 = std::make_unique<char[]>(attributename.length() + 1);
    attributename.copy(attnam2.get(), attributename.length() + 1);
    int attval = 0;
    auto retval = Getiat(&file_pointer, groupname2.get(), attnam2.get(), &attval);
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    return attval;
}

void nefis_file::set_int_attribute(const std::string &groupname, const std::string &attributename, int value)
{
    auto groupname2 = std::make_unique<char[]>(groupname.length() + 1);
    groupname.copy(groupname2.get(), groupname.length() + 1);
    auto attnam2 = std::make_unique<char[]>(attributename.length() + 1);
    attributename.copy(attnam2.get(), attributename.length() + 1);
    auto retval = Putiat(&file_pointer, groupname2.get(), attnam2.get(), &value);
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    return;
}

int nefis_file::get_maxdim_index(const std::string &groupname) const
{
    // get the maximum index from a certain group
    int max_index = 0;
    auto groupname2 = std::make_unique<char[]>(groupname.length() + 1);
    groupname.copy(groupname2.get(), groupname.length() + 1);
    auto retval = Inqmxi(&file_pointer, groupname2.get(), &max_index);
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    return max_index;
}

std::string nefis_file::get_cel_name(const std::string &grpname) const
{
    int grpndm = 0;
    int grpdms[5];
    int grpord[5];
    auto groupname2 = std::make_unique<char[]>(grpname.length() + 1);
    grpname.copy(groupname2.get(), grpname.length());
    char celnam[17]; // Nefis 5 defines celname as char[16] + \0
    if (const auto retval = Inqgrp(&file_pointer, groupname2.get(), celnam, &grpndm, grpdms, grpord); retval != 0)
    {
        throw nefis_exception(this);
    }
    return std::string(celnam);
}

std::vector<std::string> nefis_file::get_element_names(std::string grpname) const
{
    int nelems = 100;      // HACK  maximum number of elements?
    char elmnms[100 * 17]; // each field is 16 characters + '\0'
    const auto groupname2 = std::make_unique<char[]>(grpname.length() + 1);
    grpname.copy(groupname2.get(), grpname.length());
    if (const auto retval = Inqcel3(&file_pointer, groupname2.get(), &nelems, elmnms); retval != 0)
    {
        throw nefis_exception(this);
    }
    std::string tempres = std::string(elmnms);
    std::vector<std::string> element_names(nelems);
    for (size_t i = 0; i < nelems; i++)
    {
        element_names.push_back(tempres.substr(i * 17, 16));
    }
    return element_names;
}

std::string nefis_file::get_element_type(std::string elnam) const
{
    // Sizes of elmtyp, elmqty, elmunt,elmdes are hardcoded for NEFIS5
    char elmtyp[9];
    int nbytsg = 0;
    char elmqty[17];
    char elmunt[17];
    char elmdes[65];
    int elmndm = 0;
    int elmdms = 0;
    auto elname2 = std::make_unique<char[]>(elnam.length() + 1);
    elnam.copy(elname2.get(), elnam.length());
    if (auto retval = Inqelm(&file_pointer, elname2.get(), elmtyp, &nbytsg, elmqty, elmunt, elmdes, &elmndm, &elmdms);
        retval != 0)
    {
        throw nefis_exception(this);
    }
    return std::string(elmtyp);
}

std::string nefis_file::get_first_groupname() const
{
    auto grpname2 = std::make_unique<char[]>(16 + 1);
    auto defname = std::make_unique<char[]>(16 + 1);
    if (const auto retval = Inqfst(&file_pointer, grpname2.get(), defname.get()); retval != 0)
    {
        throw nefis_exception(this);
    }
    return std::string(grpname2.get());
}

void nefis_file::get_int_element(std::string grpname, std::string elmname, nefis_uindex uindex,
                                 std::vector<int> &resarray) const
{
    int count = (uindex.end - uindex.start) / uindex.step + 1;
    size_t size = resarray.size();
    if (count > size)
        throw nefis_exception("get_int_element: resarray is too small");

    std::array uindex_ = {uindex.start, uindex.end, uindex.step};
    std::array usrord = std_order;

    auto grpname2 = std::make_unique<char[]>(grpname.length() + 1);
    grpname.copy(grpname2.get(), grpname.length() + 1);
    auto elmname2 = std::make_unique<char[]>(elmname.length() + 1);
    elmname.copy(elmname2.get(), elmname.length() + 1);

    int buflen = static_cast<int>(size * 4);
    int *pt = resarray.data();
    auto retval = Getelt(&file_pointer, grpname2.get(), elmname2.get(), uindex_.data(), usrord.data(), &buflen, pt);
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
}

void nefis_file::get_int_element(std::string grpname, std::string elmname, nefis_uindex uindex_1st_dim,
                                 nefis_uindex uindex_2nd_dim, std::vector<std::vector<int>> &resarray) const
{
    int count1 = (uindex_1st_dim.end - uindex_1st_dim.start) / uindex_1st_dim.step + 1;
    int count2 = (uindex_2nd_dim.end - uindex_2nd_dim.start) / uindex_2nd_dim.step + 1;
    size_t size1 = resarray.size();
    size_t size2 = resarray[0].size();
    if (count1 > size1 || count2 > size2)
        throw nefis_exception("get_float_element: resarray is too small");

    std::array uindex = {uindex_1st_dim.start, uindex_1st_dim.end, uindex_1st_dim.step,
                         uindex_2nd_dim.start, uindex_2nd_dim.end, uindex_2nd_dim.step};
    std::array usrord = std_order;
    int buflen = static_cast<int>(size1 * size2) * 4;
    int *buffer = new int[buflen];

    auto grpname2 = std::make_unique<char[]>(grpname.length() + 1);
    grpname.copy(grpname2.get(), grpname.length() + 1);
    auto elmname2 = std::make_unique<char[]>(elmname.length() + 1);
    elmname.copy(elmname2.get(), elmname.length() + 1);
    auto retval = Getelt(&file_pointer, grpname2.get(), elmname2.get(), uindex.data(), usrord.data(), &buflen, buffer);
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    for (auto i = 0; i < size2; i++)
    {
        for (auto j = 0; j < size1; j++)
        {
            resarray[j][i] = buffer[i + j * size2];
        }
    }
}

void nefis_file::get_float_element(std::string grpname, std::string elmname, nefis_uindex uindex,
                                   std::vector<float> &resarray) const
{
    int count = (uindex.end - uindex.start) / uindex.step + 1;
    size_t size = resarray.size();
    if (count > size)
        throw nefis_exception("get_float_element: resarray is too small");

    float *pt = resarray.data();
    std::array uindex_ = {uindex.start, uindex.end, uindex.step};
    std::array usrord = std_order;
    auto buflen = int(size) * 4;
    auto grpname2 = std::make_unique<char[]>(grpname.length() + 1);
    grpname.copy(grpname2.get(), grpname.length() + 1);
    auto elmname2 = std::make_unique<char[]>(elmname.length() + 1);
    elmname.copy(elmname2.get(), elmname.length() + 1);
    auto retval = Getelt(&file_pointer, grpname2.get(), elmname2.get(), uindex_.data(), usrord.data(), &buflen, pt);
    if (retval != 0)
    {
        nefis_exception error(this);
        throw error;
    }
}

void nefis_file::get_float_element(std::string grpname, std::string elmname, nefis_uindex uindex_1st_dim,
                                   nefis_uindex uindex_2nd_dim, std::vector<std::vector<float>> &resarray,
                                   bool transpose) const
{
    int count1 = (uindex_1st_dim.end - uindex_1st_dim.start) / uindex_1st_dim.step + 1;
    int count2 = (uindex_2nd_dim.end - uindex_2nd_dim.start) / uindex_2nd_dim.step + 1;

    size_t size1 = resarray.size();
    size_t size2 = resarray[0].size();

    if (count1 > size1 || count2 > size2)
        throw nefis_exception("get_float_element: resarray is too small");

    std::array uindex = {uindex_1st_dim.start, uindex_1st_dim.end, uindex_1st_dim.step,
                         uindex_2nd_dim.start, uindex_2nd_dim.end, uindex_2nd_dim.step};
    std::array usrord = std_order;
    int buflen = int(size1 * size2);
    auto buffersize = static_cast<int>(buflen * sizeof(float));
    auto buffer = std::make_unique<float[]>(buflen);

    auto elmname2 = std::make_unique<char[]>(elmname.length() + 1);
    elmname.copy(elmname2.get(), elmname.length() + 1);
    auto grpname2 = std::make_unique<char[]>(grpname.length() + 1);
    grpname.copy(grpname2.get(), grpname.length() + 1);

    auto retval =
        Getelt(&file_pointer, grpname2.get(), elmname2.get(), uindex.data(), usrord.data(), &buffersize, buffer.get());
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    if (transpose)
    {
        for (auto i = 0; i < size1; i++)
            for (auto j = 0; j < size2; j++)
                resarray[i][j] = buffer[size2 * i + j];
    }
    else
    {
        for (auto i = 0; i < size2; i++)
            for (auto j = 0; j < size1; j++)
                resarray[j][i] = buffer[j + i * size1];
    }
}

void nefis_file::get_string_element(const std::string &grpname, const std::string &elmname, nefis_uindex uindex_1st_dim,
                                    nefis_uindex uindex_2nd_dim, int stringlength,
                                    std::vector<std::vector<std::string>> &results) const
{
    int count1 = (uindex_1st_dim.end - uindex_1st_dim.start) / uindex_1st_dim.step + 1;
    int count2 = (uindex_2nd_dim.end - uindex_2nd_dim.start) / uindex_2nd_dim.step + 1;
    std::size_t size1 = results.size();
    std::size_t size2 = results[0].size();

    if (count1 > size1 || count2 > size2)
        throw nefis_exception("get_string_element: results vector is too small");

    std::array uindex = {uindex_1st_dim.start, uindex_1st_dim.end, uindex_1st_dim.step,
                         uindex_2nd_dim.start, uindex_2nd_dim.end, uindex_2nd_dim.step};
    std::array usrord = std_order;
    if (stringlength == 0)
    {
        stringlength = get_element_size(elmname);
    }

    int buflen = static_cast<int>(size1 * size2) * stringlength;
    auto pt = std::make_unique<char[]>(buflen);

    auto elmname2 = std::make_unique<char[]>(elmname.length() + 1);
    elmname.copy(elmname2.get(), elmname.length() + 1);
    auto grpname2 = std::make_unique<char[]>(grpname.length() + 1);
    grpname.copy(grpname2.get(), grpname.length() + 1);

    auto retval =
        Getelt(&file_pointer, grpname2.get(), elmname2.get(), uindex.data(), usrord.data(), &buflen, pt.get());
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    std::string tempres = std::string(pt.get());
    for (auto j = 0; j < size2; j++)
    {
        for (auto i = 0; i < size1; i++)
        {
            // find the last charachter of the name, starting at the end and then
            // going back searching for the last space.
            int sl = stringlength - 1;
            while (
                (pt[(j + i * size2) * stringlength + sl] == ' ' || pt[(j + i * size2) * stringlength + sl] == '\0') &&
                sl > -1)
            {
                sl--;
            }
            char *temp = &pt[(j + i * size2) * stringlength];
            results[i][j] = std::string(temp, sl + 1);
        }
    }
}

void nefis_file::get_string_element(const std::string &groupname, const std::string &elementname, nefis_uindex indices,
                                    int stringlength, std::vector<std::string> &results) const
{
    int count = (indices.end - indices.start) / indices.step + 1;
    int size = static_cast<int>(results.size());
    if (count > size)
        throw std::invalid_argument("results vector is too small");

    std::array uindex_ = {indices.start, indices.end, indices.step};
    std::array usrord = std_order;
    auto grpname2 = std::make_unique<char[]>(groupname.length() + 1);
    groupname.copy(grpname2.get(), groupname.length() + 1);
    auto elmname2 = std::make_unique<char[]>(elementname.length() + 1);
    elementname.copy(elmname2.get(), elementname.length() + 1);
    if (stringlength == 0)
    {
        stringlength = get_element_size(elementname);
    }

    int buflen = size * stringlength + 1;
    auto pt = std::make_unique<char[]>(buflen);
    auto retval =
        Getels(&file_pointer, grpname2.get(), elmname2.get(), uindex_.data(), usrord.data(), &buflen, pt.get());
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    for (auto i = 0; i < size; i++)
    {
        // find the last charachter of the name, starting at the end and then going
        // back searching for the last space or null.
        int sl = stringlength - 1;
        while ((pt[i * stringlength + sl] == ' ' || pt[i * stringlength + sl] == '\0') && sl > -1)
        {
            sl--;
        }
        char *temp = &pt[i * stringlength];
        results[i] = std::string(temp, sl + 1);
    }
}

std::string nefis_file::get_next_groupname() const
{
    char *grpname2 = new char[16 + 1];
    char *defname = new char[16 + 1];
    auto retval = Inqnxt(&file_pointer, grpname2, defname);
    std::string grpname = std::string(grpname2);
    delete[] defname;
    delete[] grpname2;
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    return grpname;
}

std::string nefis_file::get_first_int_attribute(std::string grpname) const
{
    int attval = 0;
    char *grpname2 = new char[16 + 1];
    char *atname2 = new char[16 + 1];
    strcpy_s(grpname2, grpname.length() + 1, grpname.c_str());
    auto retval = Inqfia(&file_pointer, grpname2, atname2, &attval);
    std::string atname = std::string(atname2);
    delete[] atname2;
    delete[] grpname2;
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    return atname;
}

void nefis_file::flush()
{
    if (is_open())
    {
        auto retval = Flsdef(&file_pointer);
        if (retval != 0)
        {
            throw nefis_exception(this);
        }
        retval = Flsdat(&file_pointer);
        if (retval != 0)
        {
            throw nefis_exception(this);
        }
    }
}

int nefis_file::get_element_size(std::string element) const
{
    auto elmname = std::make_unique<char[]>(element.length() + 1);
    element.copy(elmname.get(), element.length() + 1);
    char elmtyp[16 + 1];
    int size_element = 0;
    char elmquant[16 + 1];
    char elmun[16 + 1];
    char elmdes[64 + 1];
    int elmdm = 5;
    int elmdms[5];
    auto retval = Inqelm(&file_pointer, elmname.get(), elmtyp, &size_element, elmquant, elmun, elmdes, &elmdm, elmdms);
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    return size_element;
}

int nefis_file::get_element_dimension(std::string element) const
{
    auto elmname = std::make_unique<char[]>(element.length() + 1);
    element.copy(elmname.get(), element.length() + 1);
    char elmtyp[16 + 1];
    int size_element = 0;
    char elmquant[16 + 1];
    char elmun[16 + 1];
    char elmdes[64 + 1];
    int elmdm = 5;
    int elmdms[5];
    auto retval = Inqelm(&file_pointer, elmname.get(), elmtyp, &size_element, elmquant, elmun, elmdes, &elmdm, elmdms);
    if (retval != 0)
    {
        throw nefis_exception(this);
    }
    return elmdms[0];
}

std::string nefis_file::get_last_error() noexcept
{
    char buffer[1024 + 1];
    auto neferr_result = Neferr(0, buffer);
    if (neferr_result != 0)
    {
        return std::string("Error in NEFERR: " + std::to_string(neferr_result));
    }
    return std::string(buffer);
}

bool nefis_file::is_open() const
{
    return file_status_open;
}
