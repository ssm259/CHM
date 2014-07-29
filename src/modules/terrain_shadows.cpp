
#include "terrain_shadows.hpp"

terrain_shadow::terrain_shadow(std::string ID)
{
    _provides->push_back("is_shadow");
    _provides->push_back("z_prime");
    this->ID = ID;
    _parallel_type = parallel::domain;
    LOG_DEBUG << "Successfully instantiated module " << this->ID;

}

void terrain_shadow::run(mesh domain, boost::shared_ptr<global> global_param)
{
    double A = global_param->solar_az();
    double E = global_param->solar_el();

    if (global_param->solar_el() < 5)
    {
        for (triangulation::Finite_faces_iterator fit = domain->finite_faces_begin(); fit != domain->finite_faces_end(); ++fit)
        {
            //interpolate the station data to the current element
            triangulation::Face_handle face = fit;
            face->set_face_data("z_prime", 0); //unshadowed
        }
        return;
    }
    //euler rotation matrix K
    arma::mat K;
    // eqns(6) & (7) in Montero
    double z0 = M_PI - A * M_PI / 180.0;
    double q0 = M_PI / 2.0 - E * M_PI / 180.0;

    K << cos(z0) << sin(z0) << 0 << arma::endr
            << -cos(q0) * sin(z0) << cos(q0) * cos(z0) << sin(q0) << arma::endr
            << sin(q0) * sin(z0) << -cos(z0) * sin(q0) << cos(q0) << arma::endr;

    for (triangulation::Finite_faces_iterator fit = domain->finite_faces_begin();
            fit != domain->finite_faces_end(); ++fit)
    {
        //interpolate the station data to the current element
        triangulation::Face_handle face = fit;
        tmp_vertex* tv = new tmp_vertex;


        arma::vec coord(3);
        triangulation::Point p[3];

        // Vertex 0
        coord(0) = face->vertex(0)->point().x();
        coord(1) = face->vertex(0)->point().y();
        coord(2) = face->vertex(0)->point().z();

        coord = K*coord;
        p[0] = triangulation::Point(coord(0), coord(1), coord(2));

        // Vertex 1
        coord(0) = face->vertex(1)->point().x();
        coord(1) = face->vertex(1)->point().y();
        coord(2) = face->vertex(1)->point().z();

        coord = K*coord;
        p[1] = triangulation::Point(coord(0), coord(1), coord(2));


        // Vertex 2
        coord(0) = face->vertex(2)->point().x();
        coord(1) = face->vertex(2)->point().y();
        coord(2) = face->vertex(2)->point().z();

        coord = K*coord;
        p[2] = triangulation::Point(coord(0), coord(1), coord(2));

        //the idea here is as follows: we are iterating over each face, however a vertex may belong to
        // >1 face, so if we blindly modify a vertex, we may doubly or triply rotate a vertex. What this logic does
        // is only rotate vertexes we haven't visited, done by checking the bool vitied flag. Then, if we have not visited a vertex
        // we push back the vertex index as well as the original x.y.z point (as we need to undo the rotation at the end)
        for (int i = 0; i < 3; i++)
        {
            if(!face->vertex(i)->info)
                face->vertex(i)->info = new vertex_flag();
            
            vertex_flag* vf = dynamic_cast<vertex_flag*>(face->vertex(i)->info);
            if (!vf->visited)
            {
                tv->v.push_back( std::make_pair(i,triangulation::Point(face->vertex(i)->point())));
                face->vertex(i)->set_point(p[i]);
                vf->visited = true; //flag as already visited so we don't doubly apply rotations
            }
        }

        face->info = tv;
    }

    //performs the actual shadow check
    for (triangulation::Finite_faces_iterator fit = domain->finite_faces_begin();
            fit != domain->finite_faces_end(); ++fit)
    {
        triangulation::Face_handle face = fit;

        double zprime_avg = CGAL::circumcenter( face->vertex(0)->point(),face->vertex(1)->point(),face->vertex(2)->point()).z();

        face->set_face_data("z_prime", zprime_avg);
    }
    
    // here we need to 'undo' the rotation we applied. It is a trade off of memor v. CPU. Frequently memory is the limitation
    // so we will do more work inorder to reuse our triangulation 
    for (triangulation::Finite_faces_iterator fit = domain->finite_faces_begin();
            fit != domain->finite_faces_end(); ++fit)
    {
        triangulation::Face_handle face = fit;
        tmp_vertex* tv = dynamic_cast<tmp_vertex*> (face->info);
        
        //iteration is over all the vertexes that need to be undone.
        for(auto itr : tv->v)
        {
            size_t i = itr.first;
            face->vertex(i)->set_point(itr.second);
            delete face->vertex(i)->info;
            face->vertex(i)->info = NULL;
        }

        delete face->info;
        face->info = NULL;
    }
}

terrain_shadow::~terrain_shadow()
{


}



