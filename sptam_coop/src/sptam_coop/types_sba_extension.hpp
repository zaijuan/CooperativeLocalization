/**
 * This file is part of S-PTAM.
 *
 * Copyright (C) 2015 Taihú Pire and Thomas Fischer
 * For more information see <https://github.com/lrse/sptam>
 *
 * S-PTAM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * S-PTAM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with S-PTAM. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:  Taihú Pire <tpire at dc dot uba dot ar>
 *           Thomas Fischer <tfischer at dc dot uba dot ar>
 *
 * Laboratory of Robotics and Embedded Systems
 * Department of Computer Science
 * Faculty of Exact and Natural Sciences
 * University of Buenos Aires
 */
#pragma once

#include <g2o/types/sba/types_sba.h>
#include <opencv2/core/eigen.hpp>

namespace g2o {

using namespace Eigen;

// monocular projection for right camera
// first two args are the measurement type, second two the connection classes
class G2O_TYPES_SBA_API EdgeProjectP2MCRight : public  BaseBinaryEdge<2, Vector2d, VertexSBAPointXYZ, VertexCam>
{
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    EdgeProjectP2MCRight();
    virtual bool read(std::istream& is);
    virtual bool write(std::ostream& os) const;

    // return the error estimate as a 2-vector
    void computeError()
    {
      // from <Point> to <Cam>
      const VertexSBAPointXYZ *point = static_cast<const VertexSBAPointXYZ*>(_vertices[0]);
      const VertexCam *cam = static_cast<const VertexCam*>(_vertices[1]);


      // calculate the projection
      Vector4d pt;
      pt.head<3>() = point->estimate();
      pt(3) = 1.0;
      const SBACam& nd = cam->estimate();
      // these should be already ok
      /* nd.setTransform(); */
      /* nd.setProjection(); */
      /* nd.setDr(); */

      Vector3d p = nd.w2n * pt;
      Vector3d pb(nd.baseline,0,0);

      // right camera px
      p = nd.Kcam*(p-pb);

      Vector2d perr;
      perr = p.head<2>()/p(2);
      //      std::cout << std::endl << "CAM   " << cam->estimate() << std::endl;
      //      std::cout << "POINT " << pt.transpose() << std::endl;
      //      std::cout << "PROJ  " << p.transpose() << std::endl;
      //      std::cout << "CPROJ " << perr.transpose() << std::endl;
      //      std::cout << "MEAS  " << _measurement.transpose() << std::endl;

      // error, which is backwards from the normal observed - calculated
      // _measurement is the measured projection
      _error = perr - _measurement;
      // std::cerr << _error.x() << " " << _error.y() <<  " " << chi2() << std::endl;
    }

    // jacobian
    virtual void linearizeOplus();
};

class G2O_TYPES_SBA_API EdgeFixedSE3PointXYZ : public g2o::BaseBinaryEdge< 3, Vector3d, VertexSBAPointXYZ, VertexCam>
{

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    EdgeFixedSE3PointXYZ(cv::Point3d fixed_position_in_Boreas){ _fixed_position_in_Boreas = fixed_position_in_Boreas; }

        void computeError(){

            const VertexSBAPointXYZ *point = static_cast<const VertexSBAPointXYZ*>(_vertices[0]);
            const VertexCam *cam_ = static_cast<const VertexCam*>(_vertices[1]);

            const Vector3d &pt = point->estimate();
            Matrix3d cam_rot = cam_->estimate().rotation().toRotationMatrix();

            Vector3d t_vec = cam_->estimate().translation();
            const Vector3d fixed_point(_fixed_position_in_Boreas.x,_fixed_position_in_Boreas.y,_fixed_position_in_Boreas.z);
            Vector3d p_err = cam_rot * fixed_point + t_vec;

            _error = pt - p_err - _measurement;
        }

    virtual bool read(std::istream& is);
    virtual bool write(std::ostream& os) const;

private:
    cv::Point3d _fixed_position_in_Boreas;
};

} // g2o
