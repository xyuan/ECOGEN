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

#include <cmath>
#include "MelKapila.h"

using namespace std;
using namespace tinyxml2;

//***************************************************************************

MelKapila::MelKapila() :m_densite(0.), m_pression(0.), m_vitesse(0), m_energie(0.), m_energieTotale(0.), m_vitesseSonFigee(0.), m_vitesseSonWood(0.) {}

//***************************************************************************

/*! Constructeur materiau a partir d une lecture au format XML
! ex : 			<melange>
!             <vitesse x="0." y="0." z="0."/>
!           </melange>
*/
MelKapila::MelKapila(XMLElement *etat, string nomFichier) :
  m_densite(0.), m_pression(0.), m_energie(0.), m_energieTotale(0.), m_vitesseSonFigee(0.), m_vitesseSonWood(0.)
{
  XMLElement *sousElement(etat->FirstChildElement("melange"));
  if (sousElement == NULL) throw ErreurXMLElement("melange", nomFichier, __FILE__, __LINE__);
  //Recuperation des donnes
  //-----------------------
  XMLError erreur;
  //vitesse
  XMLElement *vitesse(sousElement->FirstChildElement("vitesse"));
  if (vitesse == NULL) throw ErreurXMLElement("vitesse", nomFichier, __FILE__, __LINE__);
  double vitesseX(0.), vitesseY(0.), vitesseZ(0.);
  erreur = vitesse->QueryDoubleAttribute("x", &vitesseX);
  if (erreur != XML_NO_ERROR) throw ErreurXMLAttribut("x", nomFichier, __FILE__, __LINE__);
  erreur = vitesse->QueryDoubleAttribute("y", &vitesseY);
  if (erreur != XML_NO_ERROR) throw ErreurXMLAttribut("y", nomFichier, __FILE__, __LINE__);
  erreur = vitesse->QueryDoubleAttribute("z", &vitesseZ);
  if (erreur != XML_NO_ERROR) throw ErreurXMLAttribut("z", nomFichier, __FILE__, __LINE__);
  m_vitesse.setXYZ(vitesseX, vitesseY, vitesseZ);
}

//***************************************************************************

MelKapila::~MelKapila(){}

//***************************************************************************

void MelKapila::ecritMelange(ofstream &fluxFichier) const
{
  fluxFichier << m_densite << " " << m_pression << " " << m_vitesse.norme() << " ";
}

//***************************************************************************

void MelKapila::alloueEtCopieMelange(Melange **melange)
{
  *melange = new MelKapila(*this);
}

//***************************************************************************

void MelKapila::alloueYk(const int &nombrePhases)
{
  GlMel_Yk = new double[nombrePhases];
}

//***************************************************************************

void MelKapila::copieMelange(Melange &melange)
{
  m_densite = melange.getDensite();
  m_pression = melange.getPression();
  m_vitesse = melange.getVitesse();
  m_energie = melange.getEnergie();
  m_energieTotale = melange.getEnergieTotale();
  m_vitesseSonFigee = melange.getVitesseSonFigee();
  m_vitesseSonWood = melange.getVitesseSonWood();
}

//***************************************************************************

double MelKapila::calculDensite(const double *alphak, const double *rhok, const int &nombrePhases)
{
  double rho(0.);
  for(int k=0;k<nombrePhases;k++)
  {
      rho += alphak[k]*rhok[k];
  }
  return rho;
}

//***************************************************************************

double MelKapila::calculPression(const double *alphak, const double *pk, const int &nombrePhases)
{
  double p(0.);
  for(int k=0;k<nombrePhases;k++)
  {
      p += alphak[k]*pk[k];
  }
  return p;
}

//***************************************************************************

double MelKapila::calculEnergieInterne(const double *Yk, const double *ek, const int &nombrePhases)
{
  double e(0.);
  for(int k=0;k<nombrePhases;k++)
  {
      e += Yk[k]*ek[k];
  }
  return e;
}

//***************************************************************************

double MelKapila::calculVitesseSonFigee(const double *Yk, const double *ck, const int &nombrePhases)
{
  double cF(0.);
  for(int k=0;k<nombrePhases;k++)
  {
      cF += Yk[k]*ck[k]*ck[k];
  }
  return sqrt(cF);
}

//***************************************************************************

double MelKapila::calculTsat(const Eos *eosLiq, const Eos *eosVap, const double &pression, double *dTsat)
{
  //FP//TODO// ajouter un garde fou si les loi d etat ne sont pas adaptees

  double gammaL = eosLiq->getGamma();
  double pInfL = eosLiq->getPInf();
  double cvL = eosLiq->getCv();
  double e0L = eosLiq->getERef();
  double s0L = eosLiq->getSRef();

  double gammaV = eosVap->getGamma();
  double pInfV = eosVap->getPInf();
  double cvV = eosVap->getCv();
  double e0V = eosVap->getERef();
  double s0V = eosVap->getSRef();

  double A, B, C, D;
  A = (gammaL*cvL - gammaV*cvV + s0V - s0L) / (gammaV*cvV - cvV);
  B = (e0L - e0V) / (gammaV*cvV - cvV);
  C = (gammaV*cvV - gammaL*cvL) / (gammaV*cvV - cvV);
  D = (gammaL*cvL - cvL) / (gammaV*cvV - cvV);

  //Processus iteratif recherche de la temperature de saturation
  int iteration(0);
  double Tsat(0.1*B / C);
  double f(0.), df(1.);
  do {
    Tsat -= f / df; iteration++;
    if (iteration > 50) {
      erreurs.push_back(Erreurs("nombre iterations trop grand dans recherche Tsat", __FILE__, __LINE__));
      break;
    }
    f = A + B / Tsat + C*log(Tsat) - log(pression + pInfV) + D*log(pression + pInfL);
    df = C / Tsat - B / (Tsat*Tsat);
  } while (fabs(f)>1e-10);

  double dfdp = -1. / (pression + pInfV) + D / (pression + pInfL);
  if(dTsat != 0) *dTsat = -dfdp / df;
  return Tsat;
}

//***************************************************************************

void MelKapila::calculGrandeursMelange(Phase **vecPhase, const int &nombrePhases)
{
  //Densite de melange et pression
  m_densite = 0.;
  m_pression = 0.;
  for (int k = 0; k < nombrePhases; k++) {
    m_densite += vecPhase[k]->getAlpha()*vecPhase[k]->getDensite();
    m_pression += vecPhase[k]->getAlpha()*vecPhase[k]->getPression();
  }
  //Fraction massique
  for (int k = 0; k < nombrePhases; k++) {
    GlMel_Yk[k] = vecPhase[k]->getAlpha()*vecPhase[k]->getDensite() / m_densite;
  }
  //Energie interne, vitesse son figee et Wood
  m_energie = 0.;
  m_vitesseSonFigee = 0.;
  m_vitesseSonWood = 0.;
  for (int k = 0; k < nombrePhases; k++) {
    m_energie += GlMel_Yk[k] * vecPhase[k]->getEnergie();
    m_vitesseSonFigee += GlMel_Yk[k] * vecPhase[k]->getVitesseSon()*vecPhase[k]->getVitesseSon();
    m_vitesseSonWood += vecPhase[k]->getAlpha() / (vecPhase[k]->getDensite()*vecPhase[k]->getVitesseSon()*vecPhase[k]->getVitesseSon());
  }
  m_vitesseSonFigee = sqrt(m_vitesseSonFigee);
  m_vitesseSonWood = 1. / sqrt(m_densite*m_vitesseSonWood);
}

//***************************************************************************

void MelKapila::energieInterneVersEnergieTotale(vector<GrandeursPhysAdd*> &vecGPA)
{
  m_energieTotale = m_energie + 0.5*m_vitesse.normeCarre();
  for (unsigned int pa = 0; pa < vecGPA.size(); pa++) {
    m_energieTotale += vecGPA[pa]->calculEnergiePhysAdd()/m_densite; //Attention /m_densite important
  }
}

//***************************************************************************

void MelKapila::energieTotaleVersEnergieInterne(vector<GrandeursPhysAdd*> &vecGPA)
{
  m_energie = m_energieTotale - 0.5*m_vitesse.normeCarre();
  for (unsigned int pa = 0; pa < vecGPA.size(); pa++) {
    m_energie -= vecGPA[pa]->calculEnergiePhysAdd()/m_densite; //Attention /m_densite important
  }
}

//***************************************************************************

void MelKapila::calculEnergieCapillaire()
{
  //On stocke l'energie capillaire dans l'energie totale ici, utile ensuite pour le calculsEtendusPourRiemann
  m_energieTotale += - m_energie - 0.5*m_vitesse.normeCarre();
}

//***************************************************************************

void MelKapila::calculEnergieTotale()
{
  //Arrive ici, l'energie capillaire est stocke dans l'energie totale de melange. Utile pour reconstruire l'energie totale sur le bord de cellule.
  m_energieTotale += m_energie + 0.5*m_vitesse.normeCarre();
}

//***************************************************************************

void MelKapila::projection(const Coord &normale, const Coord &tangente, const Coord &binormale)
{
  m_vitesse.projection(normale, tangente, binormale);
}

//***************************************************************************

void MelKapila::projectionRepereAbsolu(const Coord &normale, const Coord &tangente, const Coord &binormale)
{
  m_vitesse.projectionRepereAbsolu(normale, tangente, binormale);
}

//****************************************************************************
//************************* ECRITURE DES DONNEES *****************************
//****************************************************************************

double MelKapila::renvoieScalaire(const int &numVar) const
{
  switch (numVar)
  {
  case 1:
    return m_densite; break;
  case 2:
    return m_pression; break;
  default:
    return 0.; break;
  }
}

//***************************************************************************

Coord* MelKapila::renvoieVecteur(const int &numVar)
{
  switch (numVar)
  {
  case 1:
    return &m_vitesse; break;
  default:
    return 0; break;
  }
}

//***************************************************************************

string MelKapila::renvoieNomScalaire(const int &numVar) const
{
  switch (numVar)
  {
  case 1:
    return "MasseVolumique_Melange"; break;
  case 2:
    return "Pression_Melange"; break;
  default:
    return "SansNom"; break;
  }
}

//***************************************************************************

string MelKapila::renvoieNomVecteur(const int &numVar) const
{
  switch (numVar)
  {
  case 1:
    return "Vitesse_Melange"; break;
  default:
    return "SansNom"; break;
  }
}

//****************************************************************************
//****************************** PARALLELE ***********************************
//****************************************************************************

int MelKapila::nombreVariablesATransmettre() const
{
  //1 scalaire + 1 vecteur : 4 variables
  return 4;
}

//***************************************************************************

void MelKapila::rempliTampon(double *tampon, int &compteur) const
{
  tampon[++compteur] = m_vitesse.getX();
  tampon[++compteur] = m_vitesse.getY();
  tampon[++compteur] = m_vitesse.getZ();
  tampon[++compteur] = m_energieTotale;
}

//***************************************************************************

void MelKapila::recupereTampon(double *tampon, int &compteur)
{
  m_vitesse.setX(tampon[++compteur]);
  m_vitesse.setY(tampon[++compteur]);
  m_vitesse.setZ(tampon[++compteur]);
  m_energieTotale = tampon[++compteur];
}

//****************************************************************************
//******************************* ORDRE 2 ************************************
//****************************************************************************

void MelKapila::calculPentesMelange(const Melange &pGauche, const Melange &pDroite, const double &distance)
{
  m_vitesse.setX((pDroite.getVitesse().getX() - pGauche.getVitesse().getX()) / distance);
  m_vitesse.setY((pDroite.getVitesse().getY() - pGauche.getVitesse().getY()) / distance);
  m_vitesse.setZ((pDroite.getVitesse().getZ() - pGauche.getVitesse().getZ()) / distance);
}

//***************************************************************************

void MelKapila::miseAZero()
{
  m_vitesse.setX(0.); m_vitesse.setY(0.); m_vitesse.setZ(0.);
}

//***************************************************************************

void MelKapila::extrapole(const Melange &pente, const double &distance)
{
  m_vitesse.setX(m_vitesse.getX() + pente.getVitesse().getX() * distance);
  m_vitesse.setY(m_vitesse.getY() + pente.getVitesse().getY() * distance);
  m_vitesse.setZ(m_vitesse.getZ() + pente.getVitesse().getZ() * distance);
}

//***************************************************************************

void MelKapila::limitePentes(const Melange &penteGauche, const Melange &penteDroite, Limiteur &limiteurGlobal)
{
  m_vitesse.setX(limiteurGlobal.limitePente(penteGauche.getVitesse().getX(), penteDroite.getVitesse().getX()));
  m_vitesse.setY(limiteurGlobal.limitePente(penteGauche.getVitesse().getY(), penteDroite.getVitesse().getY()));
  m_vitesse.setZ(limiteurGlobal.limitePente(penteGauche.getVitesse().getZ(), penteDroite.getVitesse().getZ()));
}

//****************************************************************************
//************************** ORDRE 2 PARALLELE *******************************
//****************************************************************************

int MelKapila::nombrePentesATransmettre() const
{
	return 3;
}

//***************************************************************************

void MelKapila::rempliTamponPentes(double *tampon, int &compteur) const
{
	tampon[++compteur] = m_vitesse.getX();
	tampon[++compteur] = m_vitesse.getY();
	tampon[++compteur] = m_vitesse.getZ();
}

//***************************************************************************

void MelKapila::recupereTamponPentes(double *tampon, int &compteur)
{
	m_vitesse.setX(tampon[++compteur]);
	m_vitesse.setY(tampon[++compteur]);
	m_vitesse.setZ(tampon[++compteur]);
}

//****************************************************************************
//************************** ACCESSEURS DONNEES ******************************
//****************************************************************************

double MelKapila::getDensite() const
{
  return m_densite;
}

//***************************************************************************

double MelKapila::getPression() const
{
  return m_pression;
}

//***************************************************************************

double MelKapila::getU() const { return m_vitesse.getX(); }
double MelKapila::getV() const { return m_vitesse.getY(); }
double MelKapila::getW() const { return m_vitesse.getZ(); }

//***************************************************************************

Coord MelKapila::getVitesse() const
{
  return m_vitesse;
}

//***************************************************************************

double MelKapila::getEnergie() const
{
  return m_energie;
}

//***************************************************************************

double MelKapila::getEnergieTotale() const
{
  return m_energieTotale;
}

//***************************************************************************

double MelKapila::getVitesseSonFigee() const
{
  return m_vitesseSonFigee;
}

//***************************************************************************

double MelKapila::getVitesseSonWood() const
{
  return m_vitesseSonWood;
}

//***************************************************************************

void MelKapila::setPression(const double &p) { m_pression = p; }

//***************************************************************************

void MelKapila::setVitesse(const double &u, const double &v, const double &w) { m_vitesse.setXYZ(u, v, w); }

//***************************************************************************

void MelKapila::setVitesse(const Coord &vit) { m_vitesse = vit; }

//***************************************************************************

void MelKapila::setU(const double &u) { m_vitesse.setX(u); }

//***************************************************************************

void MelKapila::setV(const double &v) { m_vitesse.setY(v); }

//***************************************************************************

void MelKapila::setW(const double &w) { m_vitesse.setZ(w); }

//***************************************************************************

void MelKapila::setEnergieTotale(double &energieTotale)
{
  m_energieTotale = energieTotale;
}

//****************************************************************************
//***************************** OPERATEURS ***********************************
//****************************************************************************

void MelKapila::changeSigne()
{
  m_densite = -m_densite;
  m_pression = -m_pression;
  m_vitesse = m_vitesse*-1.;
}

//***************************************************************************

void MelKapila::multiplieEtAjoute(const Melange &pentesMelangeTemp, const double &coeff)
{
  m_densite += pentesMelangeTemp.getDensite()*coeff;
  m_pression += pentesMelangeTemp.getPression()*coeff;
  m_vitesse += pentesMelangeTemp.getVitesse()*coeff;
}

//***************************************************************************

void MelKapila::diviser(const double &coeff)
{
  m_densite /= coeff;
  m_pression /= coeff;
  m_vitesse /= coeff;
}
