#include <iostream>
#include <labnation/smartscopeusb.h>

using namespace std;
using namespace labnation;
 
int main(int argc, char *argv[])
{
  int thisisanumber;
 
  cout<<"Please enter a number: ";
  cin>> thisisanumber;
  cin.ignore();
  cout<<"You entered: "<< thisisanumber <<"\n";
  cin.get();
  
  SmartScopeUsb ss = new SmartScopeUsb();
  cout << ss.Serial();
}