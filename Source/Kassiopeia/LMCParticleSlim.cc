/*
 * LMCParticleSlim.cc
 *
 *  Created on: Jan 13, 2017
 *      Author: nbuzinsky
 */

#include "LMCParticleSlim.hh"


namespace locust
{

    ParticleSlim::ParticleSlim() :
            fTime( -99. ),
            fPosition( -99., -99., -99. ),
            fVelocity( -99., -99., -99. ),
            fBetaVelocity( -99., -99., -99. ),
            fVelocityParallel( -99.),
            fGuidingCenterPosition( -99., -99., -99. ),
            fMagneticField( -99., -99., -99. ),
            fAlpha( -99., -99., -99. ),
            fBeta( -99., -99., -99. ),
            fMass( -99. ),
            fCharge( -99. ),
            fGamma( -99. ),
            fCyclotronFrequency( -99. ),
            fCyclotronRadius( -99. ),
            fReceiverTime( -99. ),
            fReceiverPosition( -99., -99., -99. ),
            fReceiverVector( -99., -99., -99. ),
            fReceiverDistance( -99. ),
            fReceiverDir( -99., -99., -99. )
    {
    }

    ParticleSlim::~ParticleSlim()
    {

    }

    void ParticleSlim::SetTime(double aTime)
    {
        fTime = aTime;
        return;
    }
    double ParticleSlim::GetTime()
    {
        return fTime;
    }

    void ParticleSlim::SetPosition(double X, double Y, double Z)
    {
        fPosition.SetComponents(X,Y,Z);
        return;
    }

    void ParticleSlim::SetVelocityVector(double Vx, double Vy, double Vz)
    {
        fVelocity.SetComponents(Vx,Vy,Vz);
        return;
    }

    void ParticleSlim::SetMagneticFieldVector(double Bx, double By, double Bz)
    {
        fMagneticField.SetComponents(Bx,By,Bz);
        return;
    }

    void ParticleSlim::SetMass(double aMass)
    {
        fMass = aMass;
        return;
    }

    void ParticleSlim::SetCharge(double aCharge)
    {
        fCharge = aCharge;
        return;
    }

    void ParticleSlim::SetReceiverPosition(double X, double Y, double Z)
    {
        fReceiverPosition.SetComponents(X,Y,Z);
        fReceiverVector=fReceiverPosition-fPosition;
        fReceiverDistance=fReceiverVector.Magnitude();
        fReceiverDir=fReceiverVector.Unit();
        CalculateKinematicProperties();
        return;
    }

    void ParticleSlim::SetReceiverTime(double aTime)
    {
        fReceiverTime=aTime;
        return;

    }

    void ParticleSlim::CalculateKinematicProperties()
    {
        double beta=fVelocity.Magnitude()/KConst::C();
        fGamma=1./sqrt((1.-beta)*(1.+beta));
        fCyclotronFrequency=abs(fCharge) * fMagneticField.Magnitude() / ( fMass * fGamma);
        
        fVelocityParallel=fVelocity.Dot(fMagneticField.Unit());

        KGeoBag::KThreeVector vPerp=fVelocity-fVelocityParallel*fMagneticField.Unit();

        fCyclotronRadius = fGamma * fMass * vPerp.Magnitude() / (abs(fCharge) * fMagneticField.Magnitude());

        KGeoBag::KThreeVector tCrossRadial=(fVelocity.Cross(fMagneticField)).Unit();

        if(fCharge<0)
        {
            tCrossRadial=-1.*tCrossRadial;
        }

        fGuidingCenterPosition=fPosition+fCyclotronRadius*tCrossRadial;

        fAlpha=fPosition-fGuidingCenterPosition;

        fBeta=fMagneticField.Cross(fAlpha);
        fAlpha=fAlpha.Unit();
        fBeta=fBeta.Unit();
        return;
    }

    double ParticleSlim::Interpolate(double tRetarded)
    {
        double dt=tRetarded-fTime;
        KGeoBag::KThreeVector rNewPosition=fGuidingCenterPosition + fCyclotronRadius * ( cos(fCyclotronFrequency*dt)*fAlpha + sin(fCyclotronFrequency*dt)*fBeta);
        return GetSpaceTimeInterval(tRetarded,rNewPosition);
    }


    void ParticleSlim::CalculateQuadraticCoefficients(double& A, double& B, double& C)
    {
        double c=KConst::C();
        A=1.-pow(fVelocityParallel/c,2.);
        B=-2.*(fReceiverTime-fTime)+2.*fVelocityParallel*fReceiverVector.Dot(fMagneticField.Unit())/(c*c);
        C=pow((fReceiverTime-fTime),2.)-pow(fReceiverDistance/c,2.);

        return;
    }

    double ParticleSlim::GetSpaceTimeInterval()
    {
        double s=fReceiverTime-fTime-fReceiverVector.Magnitude()/KConst::C();
        return s;
    }

    double ParticleSlim::GetSpaceTimeInterval(double tNew, KGeoBag::KThreeVector rNew)
    {
        KGeoBag::KThreeVector drNew=fReceiverPosition-rNew;
        double s=fReceiverTime-tNew-drNew.Magnitude()/KConst::C();
        return s;
    }

    double ParticleSlim::GetReceiverDistance()
    {
        return fReceiverDistance;
    }

    double ParticleSlim::NewtonStep(double tRetarded, double dtRetarded)
    {
        double c=KConst::C();
        double A,B,C;
        CalculateQuadraticCoefficients(A,B,C);

        double f_sin=A*dtRetarded*dtRetarded+B*dtRetarded+C+2.*fCyclotronRadius/(c*c)*(fReceiverVector.Dot((cos(fCyclotronFrequency*(dtRetarded+tRetarded))-cos(fCyclotronFrequency*tRetarded))*fAlpha+(sin(fCyclotronFrequency*(dtRetarded+tRetarded))-sin(fCyclotronFrequency*tRetarded))*fBeta)-fCyclotronRadius*(1.-cos(fCyclotronFrequency*dtRetarded)));
        double df_sin=2.*A*dtRetarded+B+2.*fCyclotronRadius/(c*c)*fCyclotronFrequency*(fReceiverVector.Dot(-sin(fCyclotronFrequency*(dtRetarded+tRetarded))*fAlpha+cos(fCyclotronFrequency*(dtRetarded+tRetarded))*fBeta)-fCyclotronRadius*sin(fCyclotronFrequency*dtRetarded));

        return dtRetarded-f_sin/df_sin;
    }

    double ParticleSlim::HouseHolderStep(double tRetarded, double dtRetarded)
    {
        double c=KConst::C();
        double A,B,C;
        CalculateQuadraticCoefficients(A,B,C);

        double f_sin=A*dtRetarded*dtRetarded+B*dtRetarded+C+2.*fCyclotronRadius/(c*c)*(fReceiverVector.Dot((cos(fCyclotronFrequency*(dtRetarded+tRetarded))-cos(fCyclotronFrequency*tRetarded))*fAlpha+(sin(fCyclotronFrequency*(dtRetarded+tRetarded))-sin(fCyclotronFrequency*tRetarded))*fBeta)-fCyclotronRadius*(1.-cos(fCyclotronFrequency*dtRetarded)));
        double df_sin=2.*A*dtRetarded+B+2.*fCyclotronRadius/(c*c)*fCyclotronFrequency*(fReceiverVector.Dot(-sin(fCyclotronFrequency*(dtRetarded+tRetarded))*fAlpha+cos(fCyclotronFrequency*(dtRetarded+tRetarded))*fBeta)-fCyclotronRadius*sin(fCyclotronFrequency*dtRetarded));
        double d2f_sin=2.*A - 2.*fCyclotronRadius/(c*c)*fCyclotronFrequency*fCyclotronFrequency*(fReceiverVector.Dot(cos(fCyclotronFrequency*(dtRetarded+tRetarded))*fAlpha+sin(fCyclotronFrequency*(dtRetarded+tRetarded))*fBeta)+fCyclotronRadius*cos(fCyclotronFrequency*dtRetarded));
        
        double SqrtArg=1. - 2. * d2f_sin * f_sin / (df_sin * df_sin);
        if(SqrtArg < 0)  return NewtonStep(tRetarded,dtRetarded);

        return dtRetarded - df_sin / d2f_sin * ( 1. - sqrt(SqrtArg));
    }
    

    double ParticleSlim::CalculatePower(double X, double Y, double Z)
    {
        double c=KConst::C();

        KGeoBag::KThreeVector SurfaceNormal(X,Y,Z);

        //Lorentz Equation
        KGeoBag::KThreeVector BetaDot=fCharge/fMass*fBetaVelocity.Cross(fMagneticField);

        //Lienard-Wiechert Equations
        KGeoBag::KThreeVector E=fCharge / (KConst::FourPiEps() * c *pow(1.- fReceiverDir.Dot(fBetaVelocity),3.)*fReceiverDistance ) * (c / (fGamma*fGamma*fReceiverDistance)*(fReceiverDir-fBetaVelocity) + (fReceiverDir.Cross(fReceiverDir.Cross(BetaDot))));

        KGeoBag::KThreeVector B = fReceiverDir.Cross(E)/c;

        KGeoBag::KThreeVector S =1. / KConst::MuNull()*E.Cross(B);

        double P=S.Dot(SurfaceNormal);

        return P;
    }


} /* namespace locust */
