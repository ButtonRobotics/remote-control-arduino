
#define pinX A0
int old_x , new_x;

//přiřazení čísla do intervalu
int make_interval(int value)
{
  return (value-518)/50;
}


void setup()
{
  //nastaveni komunikace a počátečních hodnot
  Serial.begin(9600);
  old_x = analogRead(pinX);
}

void loop()
{
  new_x = analogRead(pinX);

  //pošle info o pozici joystickou pouze když se změní interval a když se joystick pohne od původní pozice o víc než 25
  if(make_interval(old_x) != make_interval(new_x) && abs(old_x-new_x)>25)
  {
    old_x = new_x; 
    Serial.println0,0(make_interval(new_x));
   }
  
}
