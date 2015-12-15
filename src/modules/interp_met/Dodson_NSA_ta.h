#pragma once



#include "module_base.hpp"
#include <meteoio/MeteoIO.h>


class Dodson_NSA_ta : public module_base
{
public:
    Dodson_NSA_ta();
    ~Dodson_NSA_ta();
    void run(mesh_elem &elem, boost::shared_ptr<global> global_param);
    virtual void init(mesh domain, boost::shared_ptr<global> global_param);
    struct data : public face_info
    {
        interpolation interp;
    };
};

