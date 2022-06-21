# SERIALCMD v1.0
Nino MegaDriver  
nino@nino.com.br  
  
Simple code to connect to a serial tty, send over a command and print
the output. The goal is to make it easier when you need a simple console tool to
send a command to an arduino for example, automacally detecting its port and print
the output to STDOUT.  
  
Arduino example sketch:  
 ```

void setup(){
  // Initialize the serial port at 115200
  Serial.begin(115200);
}
  
void loop() {
  // Force output over serial by doing some flooding
  // seding a char, 0x01, that won't be printed  
  for(int i=0; i<50; i++) Serial.write(0x01);  

  // Send a welcome message (that will get printed)
  Serial.println("Say something...");   

  // Send the sync char, 0x00
  Serial.write(0x00);  

  // Wait until a command is received over serial 
  while(!Serial.available() ){ }  

  // Read it to a string
  String input = Serial.readStringUntil('\n');  

  // Process it  
  if(input.equals("hello")){  
    Serial.println("Hello, how are you?");  
  }else{  
    Serial.println("Please say something that I can understand...");  
  }  
    
  // (...) Do whatever mor you want then  
  // Send the sync again to tell it's finished  
  Serial.write(0x00);   
}  

```  
  
Expected console output:  
```
$ ./serialcmd -c hello  
Connected to "/dev/ttyUSB0"  
<< Waiting for sync...  
Say something...  
  
>> hello  
Hello, how are you?  
  
```
   
```
$ ./serialcmd -c 'klaatu barada niiiareeadsdsahdggasdas'  
Connected to "/dev/ttyUSB0"  
<< Waiting for sync...  
Say something...  
  
>> klaatu barada neiieeeeaaaarg  
Please say something that I can understand...  
  
```
 
