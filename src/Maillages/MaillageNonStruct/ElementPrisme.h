//  
//       ,---.     ,--,    .---.     ,--,    ,---.    .-. .-. 
//       | .-'   .' .')   / .-. )  .' .'     | .-'    |  \| | 
//       | `-.   |  |(_)  | | |(_) |  |  __  | `-.    |   | | 
//       | .-'   \  \     | | | |  \  \ ( _) | .-'    | |\  | 
//       |  `--.  \  `-.  \ `-' /   \  `-) ) |  `--.  | | |)| 
//       /( __.'   \____\  )---'    )\____/  /( __.'  /(  (_) 
//      (__)              (_)      (__)     (__)     (__)     
//
//  This file is part of ECOGEN.
//
//  ECOGEN is the legal property of its developers, whose names 
//  are listed in the copyright file included with this source 
//  distribution.
//
//  ECOGEN is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published 
//  by the Free Software Foundation, either version 3 of the License, 
//  or (at your option) any later version.
//  
//  ECOGEN is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with ECOGEN (file LICENSE).  
//  If not, see <http://www.gnu.org/licenses/>.

#ifndef ELEMENTPRISME_H
#define ELEMENTPRISME_H

#include "ElementNS.h"
#include "FaceTriangle.h"
#include "FaceQuadrangle.h"

class ElementPrisme : public ElementNS
{
public:
  ElementPrisme();
  virtual ~ElementPrisme();

  virtual void construitFaces(const Coord *noeuds, FaceNS **faces, int &indiceMaxFaces, int** facesTemp, int* sommeNoeudsTemp); //Plus rapide
  virtual void construitFacesSimplifie(int &iMax, int** facesTemp, int* sommeNoeudsTemp);
  virtual void attributFaceCommunicante(const Coord *noeuds, FaceNS **faces, const int &indiceMaxFaces, const int &nombreNoeudsInternes);
  virtual int compteFaceCommunicante(std::vector<int*> &faces, std::vector<int> &sommeNoeudsTemp);
  virtual int compteFaceCommunicante(int &iMax, int **faces, int *sommeNoeudsTemp);

private:
  virtual void calculVolume(const Coord *noeuds);
  virtual void calculLCFL(const Coord *noeuds);

  static const int TYPEGMSH;
  static const int NOMBRENOEUDS;
  static const int NOMBREFACES; /* ici il s'agit de quadrangles*/
  static const int TYPEVTK;
};

#endif // ELEMENTPRISME_H