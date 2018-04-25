#include <string>
using std::string ;
#include <SofaTest/Mapping_test.h>
#include <SofaSimulationGraph/DAGSimulation.h>
#include <sofa/defaulttype/VecTypes.h>
#include <sofa/defaulttype/RigidTypes.h>
#include <sofa/simulation/UpdateLinksVisitor.h>
#include <sofa/simulation/InitVisitor.h>

#include "../../../component/mapping/BeamLengthMapping.h"
#include "../../../component/BeamInterpolation.h"

#include <SofaSimulationCommon/SceneLoaderXML.h>
using sofa::simulation::SceneLoaderXML ;
using sofa::simulation::Node ;
using sofa::component::container::MechanicalObject ;

namespace sofa {
  namespace { // anonymous namespace
using namespace core;
using namespace component;
using defaulttype::Vec;
using defaulttype::Mat;


/**  Test suite for RigidMapping.
The test cases are defined in the #Test_Cases member group.
  */
template <typename _BeamLengthMapping>
struct BeamLengthMappingTest : public Mapping_test<_BeamLengthMapping>
{

    typedef _BeamLengthMapping BeamLengthMapping;
    typedef Mapping_test<BeamLengthMapping> Inherit;

    typedef typename BeamLengthMapping::In InDataTypes;
    typedef typename InDataTypes::VecCoord InVecCoord;
    typedef typename InDataTypes::VecDeriv InVecDeriv;
    typedef typename InDataTypes::Coord InCoord;
    typedef typename InDataTypes::Deriv InDeriv;
    typedef container::MechanicalObject<InDataTypes> InMechanicalObject;
    typedef typename InMechanicalObject::ReadVecCoord  ReadInVecCoord;
    typedef typename InMechanicalObject::WriteVecCoord WriteInVecCoord;
    typedef typename InMechanicalObject::WriteVecDeriv WriteInVecDeriv;
    typedef typename InCoord::Pos Translation;
    typedef typename InCoord::Rot Rotation;
    typedef typename InDataTypes::Real InReal;
    typedef Mat<InDataTypes::spatial_dimensions,InDataTypes::spatial_dimensions,InReal> RotationMatrix;


    typedef typename BeamLengthMapping::Out OutDataTypes;
    typedef typename OutDataTypes::VecCoord OutVecCoord;
    typedef typename OutDataTypes::VecDeriv OutVecDeriv;
    typedef typename OutDataTypes::Coord OutCoord;
    typedef typename OutDataTypes::Deriv OutDeriv;
    typedef container::MechanicalObject<OutDataTypes> OutMechanicalObject;
    typedef typename OutMechanicalObject::WriteVecCoord WriteOutVecCoord;
    typedef typename OutMechanicalObject::WriteVecDeriv WriteOutVecDeriv;
    typedef typename OutMechanicalObject::ReadVecCoord ReadOutVecCoord;
    typedef typename OutMechanicalObject::ReadVecDeriv ReadOutVecDeriv;


    BeamLengthMapping* beamLengthMapping;

    BeamLengthMappingTest()
    {
        this->errorFactorDJ = 200;

        beamLengthMapping = static_cast<BeamLengthMapping*>( this->mapping );

        // beamLengthMapping::getJs is not yet implemented
        this->flags &= ~Inherit::TEST_getJs;

        // beamLengthMapping::getK is not yet implemented
        this->flags &= ~Inherit::TEST_getK;

        // beamLengthMapping::applyDJT is not yet implemented
        this->flags &= ~Inherit::TEST_applyDJT;

    }

    /** @name Test_Cases
      verify the computation of the beam lengths and the derivatives
      */
    ///@{
    /** Two frames are placed + line topology + and the mapping is constructed
    */

    bool testCase1()
    {
        const int Nout=1; // WARNING this number has to be changed to test with more than one beam !!
        const int Nin=2;

        string scene =
                "<?xml version='1.0'?>"
                ""
                "<Node 	name='Root' gravity='0 0 0' time='0' animate='0'>"
                "   			<EulerImplicit rayleighStiffness='0.08' rayleighMass='0.08' printLog='false' />"
                "               <CGLinearSolver iterations='100' threshold='1e-10' tolerance='1e-15' />"
                "               <Mesh name='meshSuture' edges='0 1' />"
                "               <MechanicalObject template='Rigid' name='DOFs' showIndices='0' position='0 0 0 0 0 0 1   1 0 0 0 0 0 1'/>"
                "               <BeamInterpolation name='Interpol' radius='0.1'/>"
                "               <Node name='Map' > "
                "                      <MechanicalObject template='Vec1d' name='mappedDOFs' position='1.0'  />"
                "                      <BeamLengthMapping name='beamLMap' interpolation='@Interpol' input='@DOFs' output='@mappedDOFs' />"
                "               </Node>"
                "</Node> " ;
        this->root = SceneLoaderXML::loadFromMemory ( "testCase1", scene.c_str(), scene.size());



        //std::cout<<"*******************  Get the mapping ";
        BeamLengthMapping* lengthMapping;
        this->root->getTreeObject(lengthMapping);
        this->mapping = lengthMapping;
        this->deltaRange.first= 100;
        this->deltaRange.second= 100000;


        MechanicalObject<Rigid3>* FromModel = nullptr;
        this->root->getTreeObject(FromModel);
        this->inDofs = FromModel;

        MechanicalObject<Vec1Types>* ToModel = nullptr;
        this->root->getTreeObject(ToModel);
        this->outDofs= ToModel;

        const Data<InVecCoord>& dataInX = *FromModel->read(VecCoordId::position());
        const InVecCoord& xin = dataInX.getValue();

        const Data<OutVecCoord>& dataOutX = *ToModel->read(VecCoordId::position());
        const OutVecCoord& xout = dataOutX.getValue();

        std::cout<<" x in  = "<<xin<<std::endl;

        std::cout<<" x out  = "<<xout<<std::endl;



        // new values in (curve) pos=  0.912591 -0.419907 0 0 0 -0.283387 0.959006
        InVecCoord xin_new(Nin);
        xin_new[1][0]=0.912591;
        xin_new[1][1]=-0.419907;
        xin_new[1][5]= -0.283387;
        xin_new[1][6]= 0.959006;

        // expected mapped values
        OutVecCoord expectedChildCoords(Nout);
        expectedChildCoords[0][0] = 1.0197604809762477;



        return this->runTest(xin,xout,xin_new,expectedChildCoords);


    }


};

// Define the list of types to instanciate. We do not necessarily need to test all combinations.
using testing::Types;
typedef Types<
mapping::_beamlengthmapping_::BeamLengthMapping<defaulttype::Rigid3dTypes,defaulttype::Vec1dTypes>
//,mapping::_beamlengthmapping_::BeamLengthMapping<defaulttype::Rigid3fTypes,defaulttype::Vec1fTypes>
> DataTypes; // the types to instanciate.

// Test suite for all the instanciations
TYPED_TEST_CASE(BeamLengthMappingTest, DataTypes);
// first test case
TYPED_TEST( BeamLengthMappingTest , testCase1 )
{
    // child coordinates given directly in parent frame
    ASSERT_TRUE(this->testCase1());
}


  }// anonymous namespace
  }//sofa
