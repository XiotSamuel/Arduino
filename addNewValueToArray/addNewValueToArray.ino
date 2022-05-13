void setup() {
  
  Serial.begin(115200);
  // put your setup code here, to run once:
  int meshMap[200];
  size_t n = sizeof(meshMap)/sizeof(meshMap[0]);

  int test =100;
  int pointer =0;

  boolean newValue=false;
  
  Serial.print("the size of meshMap ");
  Serial.println(n);
  
  for (int i=0;i<200;i++){
    if (meshMap[i]==test){
        newValue=false;
    }
  }

  if (newValue=true){
        pointer = pointer+1;
        meshMap[pointer]=test;
        Serial.print("new element add to positon ");
        Serial.println(pointer);
        Serial.print("value is ");
        Serial.println(meshMap[pointer]);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
