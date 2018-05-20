/// FROM PROCESSING TO ARDUINO
/*
import processing.serial.*;

Serial myPort;  // Create object from Serial class
String val;

void setup() 
{
  size(200,200); //make our canvas 200 x 200 pixels big
  String portName = Serial.list()[1]; //change the 0 to a 1 or 2 etc. to match your port
  myPort = new Serial(this, portName, 9600);
}
void draw() {
  if (mousePressed == true) 
  {                           //if we clicked in the window
   myPort.write('1');         //send a 1
   println("1");   
  } else 
  {                           //otherwise
  myPort.write('0');          //send a 0
  println("0");
  }
  if ( myPort.available() > 0) 
  {  // If data is available,
  val = myPort.readStringUntil('\n');         // read it and store it in val
  println("9");
  } 
println(val); //print it out in the console
}

*/
/// FROM ARDUINO TO PROCESSING
/*
import processing.serial.*;
Serial myPort;  // Create object from Serial class
String val;     // Data received from the serial port
void setup()
{
  // I know that the first port in the serial list on my mac
  // is Serial.list()[0].
  // On Windows machines, this generally opens COM1.
  // Open whatever port is the one you're using.
  String portName = Serial.list()[1]; //change the 0 to a 1 or 2 etc. to match your port
  myPort = new Serial(this, portName, 115200);
}
  void draw()
{
  if ( myPort.available() > 0) 
  {  // If data is available,
  val = myPort.readStringUntil('\n');         // read it and store it in val
  } 
println(val); //print it out in the console
}
*/

///CODE FOR CAMERA SNAPSHOT | SEND AND RECEIVE


import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.awt.event.KeyListener;
import java.awt.Robot;
import java.io.*;


import processing.serial.*;
Serial myPort;
String filename = "photo.jpg";
String val;
int valeur = 0;
int i = 0;
char send = '0';
byte[] photo = {};
Boolean readData = false;
Boolean initialisation = false;
PImage captureImage;
void setup(){
 size(640,480);
 println( Serial.list() ); 
 myPort = new Serial( this, Serial.list()[1], 115200 );   //Serial.list()[3][ch12399][ch29872][ch22659][ch12395][ch21512][ch12431][ch12379][ch12390][ch22793][ch26356][ch12377][ch12427][ch12371][ch12392][ch12290]
} 

void draw(){
 byte[] buffer = new byte[128];
 
  // myPort.write(0);
  if(initialisation == false)
  {
      myPort.write('1');
      println("initialisation");
      delay(1000);
  }
  
  while( myPort.available() > 0 && readData == false)    {
    // print( "COM Data: " );
     //println( myPort.readString() );
     val = myPort.readStringUntil('\n');
     println(val);
     valeur = Integer.parseInt(val.trim());
     println(valeur);
     readData = true;
     initialisation = true;
     println( "Waiting for data ..." );
   }
  
 if( readData )  
 { 
     while( myPort.available() > 0 )    {
       int readBytes = myPort.readBytes( buffer );
       print( "Read " );
       print( readBytes );
       println( " bytes ..." );
       for( int i = 0; i < readBytes; i++ )      {
         photo = append( photo, buffer[i] );
       }
     }
   
       if( myPort.available() == 0 && photo.length >= valeur) {
       if( photo.length > 0 ) {
       readData = false;
       print( "Writing to disk " );
       print( photo.length );
       println( " bytes ..." );
       saveBytes( filename, photo );
       println( "DONE!" );
       photo = new byte[0];
       try {
       Thread.sleep(500);
       } catch(InterruptedException ie) {
                    ie.printStackTrace();
                }
       captureImage = loadImage(filename);
       image(captureImage, 0, 0);
   
   
        class AfficheurFlux implements Runnable {
    
            private final InputStream inputStream;
        
            AfficheurFlux(InputStream inputStream) {
                this.inputStream = inputStream;
            }
        
            private BufferedReader getBufferedReader(InputStream is) {
                return new BufferedReader(new InputStreamReader(is));
            }
        
            @Override
            public void run() {
                BufferedReader br = getBufferedReader(inputStream);
                String ligne = "";
                try {
                    while ((ligne = br.readLine()) != null) {
                        System.out.println(ligne);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        String CHEMIN = "c:\\Users\\Alexandre\\Desktop\\Arduino\\Code\\sketch_170502a\\Test opencv.exe";

        System.out.println("Début du programme");
        try {
            String[] commande = {CHEMIN};
            Process p = Runtime.getRuntime().exec(commande);
            AfficheurFlux fluxSortie = new AfficheurFlux(p.getInputStream());
            AfficheurFlux fluxErreur = new AfficheurFlux(p.getErrorStream());

            new Thread(fluxSortie).start();
            new Thread(fluxErreur).start();

            p.waitFor();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        System.out.println("Fin du programme");

        String chaine="";
        String fichier ="test.txt";
        
        //lecture du fichier texte  
        try{
          InputStream ips=new FileInputStream(fichier); 
          InputStreamReader ipsr=new InputStreamReader(ips);
          BufferedReader br=new BufferedReader(ipsr);
          String ligne;
          while ((ligne=br.readLine())!=null){
          println(ligne);
          char send = ligne.charAt(0);
          myPort.write('1');
          /*for(i=0;i>10;i++)
          {*/
          //delay(1000);
            
            myPort.write(send);
            readData = false;
            //delay(100);
          //}
            chaine+=ligne+"\n";
          }
          br.close(); 
        }    
        catch (Exception e){
          System.out.println(e.toString());
        }

     }
   }
   
   
 }  else  {
   while( myPort.available() > 0 )    {
    // print( "COM Data: " );
     println( myPort.readString() );
     val = myPort.readStringUntil('\n');
   }
 }
}
/*
void keyPressed(){
   else {
   readData = true;
   myPort.write(0);
   println( "Waiting for data ..." );
 }
}*/