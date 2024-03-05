#include <unistd.h>
#include <conserial.h>
#include <iostream>
#include <ctime>
using namespace hwe;
using namespace std;
Conserial example;
void TestSpeedMessage(){
    api::SLevelsResponse response;

   example.SetLaserState(1);
   usleep(50000);
   response = example.GetSignalLevels();
   cout << response.signal_.h_ << " "<<response.signal_.v_ <<endl;
   example.SetLaserState(0);
   int count =0;
   while(count <20) {
   response = example.GetSignalLevels();
   cout << response.signal_.h_ << " "<<response.signal_.v_ <<endl;
   usleep(15000);
   count++;
   }

   cout<< "Привет"<<endl;
   example.SetLaserState(0);
   usleep(50000);
   response = example.GetSignalLevels();
   cout << response.signal_.h_ << " "<<response.signal_.v_ <<endl;
   example.SetLaserState(1);

   count =0;
   while(count <6) {
   response = example.GetSignalLevels();
   cout << response.signal_.h_ << " "<<response.signal_.v_ <<endl;
   usleep(50000);
   count++;
   }

   usleep(300000);
   response = example.GetSignalLevels();
   cout << response.signal_.h_ << " "<<response.signal_.v_ <<endl;
};

void RunAllFunction(){
    cout << "SetLaserState 1"<<endl;
    example.SetLaserState(1);
    cout << "GetLaserState"<<endl;
    example.GetLaserState();

    cout << "SetLaserPower 100"<<endl;
    example.SetLaserPower(100);
    cout << "GetLaserPower"<<endl;
    example.GetLaserPower();
    cout << "SetLaserPower 50"<<endl;
    example.SetLaserPower(50);
    cout << "GetLaserPower"<<endl;
    example.GetLaserPower();

    cout << "Init"<<endl;
    example.Init();

    cout << "InitByButtons"<<endl;
    example.InitByButtons({1,1,1,1});


    cout << "InitByPD"<<endl;
    example.InitByPD();
    cout << "GetInintParams"<<endl;
    example.GetInitParams();


    cout << "SetPlatesAngles"<<endl;
    example.SetPlatesAngles({0,0,0,0});

    cout << "GetPlatesAngles"<<endl;
    example.GetPlatesAngles();

    cout << "GetSignalLevels"<<endl;
    example.GetSignalLevels();
    cout << "GetLightNoises"<<endl;
    example.GetLightNoises();

    cout << "GetRotateStep"<<endl;
    example.GetRotateStep();
    cout << "GetHardwareState"<<endl;
    //example.GetHardwareState();
    cout << "RunTest"<<endl;
    example.RunTest();

    cout << "SetLaserState"<<endl;
    example.SetLaserState(0);
    cout << "GetLaserState"<<endl;
    example.GetLaserState();

};

void Scan (int platesID, int power){
    float i = 0;
    while (i < 360) {
        switch (platesID) {
        case 1: example.Sendmessage({i,0,0,0}, power);break;
        case 2: example.Sendmessage({0,i,0,0}, power);break;
        case 3: example.Sendmessage({0,0,i,0}, power);break;
        case 4: example.Sendmessage({0,0,0,i}, power);break;
        default:  break;
        }
        i= i+0.3;
    }
}


int main()
{

//    example.GetInitParams();
    //example.GetSignalLevels();
//    sleep(10);
//    example.SetLaserState(0);
 example.Init();
 example.SetLaserState(1);
//    example.SetLaserState(0);
//    example.SetLaserState(1);
//    example.SetLaserState(0);
    return 0;
}

