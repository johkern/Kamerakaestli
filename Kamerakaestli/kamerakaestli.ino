////////////////////////////////////////
//                                    //
//        Kamera_KastliV1.2           //
//    Raphael Hahn + Johannes Kern    //
//        Kamera_KästliV1.2           //
//                                    //
////////////////////////////////////////

#include <ShiftOutX.h>
#include <ShiftPinNo.h>
#include <LiquidCrystal.h>
#include <MenuBackend.h>    //MenuBackend library - copyright by Alexander Brevig
							// ergänzt und erweitert

boolean develop = true;
// Shiftregister
byte SERIN=7;
byte CLK=4;
byte RCLK=2;

shiftOutX cam_interface(RCLK, SERIN, CLK, MSBFIRST, 4);

// Lcd
byte RS=13;
byte Enable=12;
byte D4=11;
byte D5=10;
byte D6=9;
byte D7=8;

LiquidCrystal lcd(RS, Enable, D4, D5, D6, D7);

// Taster
const byte cmd_4=19;
const byte cmd_3=18;
const byte cmd_2=17;
const byte cmd_1=16;

const byte buttonPinLeft = cmd_1;      // pin for the Up button
const byte buttonPinRight = cmd_4;    // pin for the Down button
const byte buttonPinUp = cmd_2;     // pin for the Esc button
const byte buttonPinDown = cmd_3;   // pin for the Enter button

byte lastButtonPushed = 0;

int lastButtonRightState = HIGH;   // the previous reading from the Enter input pin
int lastButtonLeftState = HIGH;   // the previous reading from the Esc input pin
int lastButtonUpState = HIGH;   // the previous reading from the Left input pin
int lastButtonDownState = HIGH;   // the previous reading from the Right input pin

long lastRightDebounceTime = 0;  // the last time the output pin was toggled
long lastLeftDebounceTime = 0;  // the last time the output pin was toggled
long lastUpDebounceTime = 0;  // the last time the output pin was toggled
long lastDownDebounceTime = 0;  // the last time the output pin was toggled

unsigned int debounceDelay = 80;    // the debounce time

//Programm Register
unsigned long gap_time = 0;
unsigned int ctr_max = 0;

unsigned int loop_click_duration=0;          //Auslösezeit
boolean scharf = false;              //einmaliges Scharfstellen etnspricht true



// einstellbare Werte
// u fuer user
//====================================

// invo
	byte u_invo_rec_time_h = 0;
	byte u_invo_rec_time_min = 0;

	byte u_invo_play_time_min = 0;
	byte u_invo_play_time_s = 0;

	byte u_invo_fps = 25;

	byte u_invo_gap_time_ms = 0;
	byte u_invo_gap_time_s = 0;

	long u_invo_gap_time = 500;  //ms
	unsigned int u_invo_ctr_max = 10;
// bulb
	byte u_bulb_exp_time_min = 0;
	byte u_bulb_exp_time_s = 0;
	byte u_bulb_exp_time_ms = 0;

	byte u_bulb_wait_time_s = 0;



//====================================

//MenuVariablen
MenuBackend menu = MenuBackend(menuUsed,menuChanged);

//initialize menuitems
MenuItem intervalvo = MenuItem("Intervalvo");
MenuItem invo_rec_time_1 = MenuItem("Invo_rec_time_1");
MenuItem invo_rec_time_h = MenuItem("Invo_rec_time_h");
MenuItem invo_rec_time_min = MenuItem("Invo_rec_time_min");
MenuItem invo_play_time_min = MenuItem("Invo_play_time_min");
MenuItem invo_play_time_s = MenuItem("Invo_play_time_s");
MenuItem invo_fps_set = MenuItem("Invo_fps_set");
MenuItem invo_go_1 = MenuItem("Invo_go_1");        // auch unter invo_ctr_max
MenuItem invo_go_2 = MenuItem("Invo_go_2");        // auch unter invo_ctr_max

MenuItem invo_gap_time_1 = MenuItem("Invo_gap_time_1");
MenuItem invo_gap_time_s = MenuItem("Invo_gap_time_s");
MenuItem invo_gap_time_ms = MenuItem("Invo_gap_time_ms");
MenuItem invo_ctr_max = MenuItem("Invo_ctr_max");

MenuItem bulb_1 = MenuItem("Bulb_1");
MenuItem bulb_exp_time_1 = MenuItem("Bulb_exp_time_1");
MenuItem bulb_exp_time_min = MenuItem("Bulb_exp_time_min");
MenuItem bulb_exp_time_s = MenuItem("Bulb_exp_time_s");
MenuItem bulb_exp_time_ms = MenuItem("Bulb_exp_time_ms");
MenuItem bulb_wait_time_1 = MenuItem("Bulb_wait_time_1");
MenuItem bulb_wait_time_s = MenuItem("Bulb_wait_time_s");
MenuItem bulb_go = MenuItem("Bulb_go");

MenuItem combi_1 = MenuItem("Combi_1");

MenuItem settings_1 = MenuItem("Settings_1");
MenuItem settings_light = MenuItem("Settings_light");
MenuItem settings_moduls = MenuItem("Settings_moduls");
MenuItem settings_credits = MenuItem("Settings_credits");
MenuItem settings_variables = MenuItem("Settings_variables");

//User Char
byte letter_ae[8] = {0b01010,0b00000,0b01110,
		0b00001,0b01111,0b10001,0b01111};
byte letter_oe[8] = {
		0b01010,0b00000,0b01110,0b10001,
		0b10001,0b10001,0b01110};
byte load_1[8] = {0b11000,0b11000,0b00000,
		0b00000,0b00000,0b00011,0b00011};
byte load_2[8] = {0b00011,0b00011,0b00000,
		0b00000,0b00000,0b11000,0b11000};
byte load_3[8] = {0b00000,0b00000,0b10001,
		0b10001,0b10001,0b00000,0b00000};


void printNumber(byte number) {
	if (number >9 ) {
	lcd.print(number);
	} else {
		lcd.print("0");
		lcd.print(number);
	}
}

void readButtons() {  //read buttons status
	int reading;
	int buttonRightState=HIGH;             // the current reading from the Enter input pin
	int buttonLeftState=HIGH;             // the current reading from the input pin
	int buttonUpState=HIGH;             // the current reading from the input pin
	int buttonDownState=HIGH;             // the current reading from the input pin

	//Enter button
	// read the state of the switch into a local variable:
	reading = digitalRead(buttonPinRight);

	// check to see if you just pressed the enter button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:

	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonRightState) {
		// reset the debouncing timer
		lastRightDebounceTime = millis();
	}

	if ((millis() - lastRightDebounceTime) > debounceDelay*2) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:
		buttonRightState=reading;
		lastRightDebounceTime=millis();
	}

	// save the reading.  Next time through the loop,
	// it'll be the lastButtonState:
	lastButtonRightState = reading;


	//Esc button
	// read the state of the switch into a local variable:
	reading = digitalRead(buttonPinLeft);

	// check to see if you just pressed the Down button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:

	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonLeftState) {
		// reset the debouncing timer
		lastLeftDebounceTime = millis();
	}

	if ((millis() - lastLeftDebounceTime) > debounceDelay*2) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:
		buttonLeftState = reading;
		lastLeftDebounceTime=millis();
	}

	// save the reading.  Next time through the loop,
	// it'll be the lastButtonState:
	lastButtonLeftState = reading;


	//Down button
	// read the state of the switch into a local variable:
	reading = digitalRead(buttonPinDown);

	// check to see if you just pressed the Down button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:

	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonDownState) {
		// reset the debouncing timer
		lastDownDebounceTime = millis();
	}

	if ((millis() - lastDownDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:
		buttonDownState = reading;
		lastDownDebounceTime =millis();
	}

	// save the reading.  Next time through the loop,
	// it'll be the lastButtonState:
	lastButtonDownState = reading;


	//Up button
	// read the state of the switch into a local variable:
	reading = digitalRead(buttonPinUp);

	// check to see if you just pressed the Down button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:

	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonUpState) {
		// reset the debouncing timer
		lastUpDebounceTime = millis();
	}

	if ((millis() - lastUpDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:
		buttonUpState = reading;
		lastUpDebounceTime=millis();
		;
	}

	// save the reading.  Next time through the loop,
	// it'll be the lastButtonState:
	lastButtonUpState = reading;

	//records which button has been pressed
	if (buttonRightState==LOW){
		lastButtonPushed=buttonPinRight;

	}
	else if(buttonLeftState==LOW){
		lastButtonPushed=buttonPinLeft;

	}
	else if(buttonDownState==LOW){
		lastButtonPushed=buttonPinDown;

	}
	else if(buttonUpState==LOW){
		lastButtonPushed=buttonPinUp;

	}
	else{
		lastButtonPushed=0;
	}
}

byte minmax (byte number,byte min,byte max) {
	if (number == max+1) {
		number = min;
	} else if ((min == 0 && number == (255))|| (number < min)){
		number = max;
	}
	return number;
}

void setNumber (byte number, byte x, byte y) {
	lcd.setCursor(x,y);
	printNumber(number);
	lcd.setCursor(x+1,y);
	lcd.blink();
}

void inc_dec_menu(String Name, short sprung) {
	if(Name == "Invo_rec_time_h") {
		u_invo_rec_time_h = u_invo_rec_time_h + sprung;
		u_invo_rec_time_h = minmax(u_invo_rec_time_h,0,99);
		setNumber(u_invo_rec_time_h,0,1);

	}else if(Name == "Invo_rec_time_min") {
		u_invo_rec_time_min= u_invo_rec_time_min + sprung;
		u_invo_rec_time_min = minmax(u_invo_rec_time_min, 0, 59);
		setNumber(u_invo_rec_time_min,3,1);

	}else if(Name == "Invo_play_time_min") {
		u_invo_play_time_min = u_invo_play_time_min + sprung;
		u_invo_play_time_min = minmax(u_invo_play_time_min, 0, 59);
		setNumber(u_invo_play_time_min,0,1);

	}else if(Name == "Invo_play_time_s") {
		u_invo_play_time_s = u_invo_play_time_s + sprung;
		u_invo_play_time_s = minmax(u_invo_play_time_s, 0, 59);
		setNumber(u_invo_play_time_s,3,1);

	}else if(Name == "Invo_fps_set") {
		u_invo_fps = u_invo_fps + sprung;
		u_invo_fps = minmax(u_invo_fps, 0, 99);
		setNumber(u_invo_fps,0,1);

	} else if (Name == "Invo_gap_time_s") {
		u_invo_gap_time_s = u_invo_gap_time_s + sprung;
		u_invo_gap_time_s = minmax(u_invo_gap_time_s,0,99);
		setNumber(u_invo_gap_time_s,0,1);

	}else if (Name == "Invo_gap_time_ms") {
		u_invo_gap_time_ms = u_invo_gap_time_ms + sprung;
		u_invo_gap_time_ms = minmax(u_invo_gap_time_ms,0,99);
		setNumber(u_invo_gap_time_ms,3,1);

	}else if (Name == "Invo_ctr_max") {
		u_invo_ctr_max = u_invo_ctr_max + sprung;
		u_invo_ctr_max = minmax(u_invo_ctr_max,1,255);
		setNumber(u_invo_ctr_max,0,1);

	}else if (Name == "Bulb_exp_time_min") {
		u_bulb_exp_time_min = u_bulb_exp_time_min + sprung;
		u_bulb_exp_time_min = minmax(u_bulb_exp_time_min,0,99);
		setNumber(u_bulb_exp_time_min,0,1);

	}else if (Name == "Bulb_exp_time_s") {
		u_bulb_exp_time_s = u_bulb_exp_time_s + sprung;
		u_bulb_exp_time_s = minmax(u_bulb_exp_time_s,0,59);
		setNumber(u_bulb_exp_time_s,3,1);

	}else if (Name == "Bulb_exp_time_ms") {
		u_bulb_exp_time_ms = u_bulb_exp_time_ms + sprung;
		u_bulb_exp_time_ms = minmax(u_bulb_exp_time_ms,0,99);
		setNumber(u_bulb_exp_time_ms,6,1);

	}else if (Name == "Bulb_wait_time_s") {
		u_bulb_wait_time_s = u_bulb_wait_time_s + sprung;
		u_bulb_wait_time_s = minmax(u_bulb_wait_time_s,0,99);
		setNumber(u_bulb_wait_time_s,0,1);

	}else {
		if (sprung > 0) {
			menu.moveDown();
		} else {
			menu.moveUp();
		}

	}


}
void navigateMenus() {
	MenuItem currentMenu=menu.getCurrent();

	switch (lastButtonPushed){

	case buttonPinRight:
		if(!(currentMenu.moveRight())){  //if the current menu has a child and has been pressed enter then menu navigate to item below
			menu.use();
		}
		else{  //otherwise, if menu has no child and has been pressed enter the current menu is used
			menu.moveRight();
			//lcd.noBlink();
		}
		break;

	case buttonPinLeft:
		menu.moveLeft();  //back to main
		lcd.noBlink();
		break;

	case buttonPinDown:
		inc_dec_menu(currentMenu.getName(), 1);
		break;

	case buttonPinUp:
		inc_dec_menu(currentMenu.getName(), -1);
		break;
	}

	lastButtonPushed=0; //reset the lastButtonPushed variable
}

void menuChanged(MenuChangeEvent changed){
	MenuItem newMenuItem=changed.to; //get the destination menu
	String name = changed.to.getName();
	if (changed.to == menu.getRoot()) {
		lcd.clear();
		lcd.print("Hauptmenu");

	} else if (name == "Invo_rec_time_1") {
		lcd.clear();
		lcd.print("Abstand ausrechnen");

	}else if (name == "Invo_rec_time_h") {
		lcd.clear();
		lcd.print("set rectime:");
		lcd.setCursor(0,1);
		printNumber(u_invo_rec_time_h);
		lcd.print(":");
		printNumber(u_invo_rec_time_min);
		lcd.print(" hh:mm");
		lcd.setCursor(1,1);
		lcd.blink();

	}else if (name == "Invo_rec_time_min") {
		lcd.blink();
		lcd.setCursor(4,1);
	}else if (name == "Invo_play_time_min") {
		lcd.clear();
		lcd.print("set playtime:");
		lcd.setCursor(0,1);
		printNumber(u_invo_play_time_min);
		lcd.print(":");
		printNumber(u_invo_play_time_s);
		lcd.print(" mm:ss");
		delay(400);
		lcd.setCursor(1,1);
		lcd.blink();

	}else if (name == "Invo_play_time_s") {
		lcd.blink();
		lcd.setCursor(4,1);

	}else if (name == "Invo_fps_set") {
		lcd.clear();
		lcd.print("set fps:");
		lcd.setCursor(0,1);
		printNumber(u_invo_fps);
		lcd.print(" 1/s");
		lcd.setCursor(1,1);
		lcd.blink();
		lcd.blink();
		lcd.setCursor(1,1);

	}else if (name == "Invo_go_1") {
		ctr_max = (u_invo_play_time_min*60+u_invo_play_time_s)* u_invo_fps;

		gap_time = (u_invo_rec_time_h*60 + u_invo_rec_time_min)*60000/ctr_max;

		lcd.clear();
		lcd.print("ctr_max:");
		lcd.print(ctr_max);
		lcd.setCursor(0,1);
		lcd.print("gaptime:");
		if (gap_time > 99999) { // ab als 100 sekunden
			lcd.print(gap_time/60000);
			lcd.print("min");
			lcd.print((gap_time-(gap_time/60000*60000))/1000);
			lcd.print("s");
		} else if (gap_time > 2000){ // ab 2 sekunden
			lcd.print(gap_time/1000);
			lcd.print(",");
			lcd.print(gap_time/100);
			lcd.print("s");
		}   else {
			lcd.print(gap_time);
			lcd.print("ms");
		}

	}else if (name == "Invo_gap_time_1") {
		lcd.clear();
		lcd.print("Abstand angeben");

	}else if (name == "Invo_gap_time_s") {
		lcd.setCursor(0,1);
		printNumber(u_invo_gap_time_s);
		lcd.print(":");
		printNumber(u_invo_gap_time_ms);
		lcd.print(" ss:ms");
		setNumber(u_invo_gap_time_s,0,1);

	}else if (name == "Invo_gap_time_ms") {
		setNumber(u_invo_gap_time_ms,3,1);

	}else if (name == "Invo_ctr_max") {
		lcd.clear();
		lcd.print("Anzahl Fotos:");
		setNumber(u_invo_ctr_max,0,1);

	}else if (name == "Invo_go_2") {
		ctr_max= u_invo_ctr_max;
		gap_time = u_invo_gap_time_ms*10+u_invo_gap_time_s*1000;

		lcd.clear();
		lcd.print("ctr_max:");
		lcd.print(ctr_max);
		lcd.setCursor(0,1);
		lcd.print("gaptime:");
		if (gap_time > 99999) { // ab als 100 sekunden
			lcd.print(gap_time/60000);
			lcd.print("min");
			lcd.print((gap_time-(gap_time/60000*60000))/1000);
			lcd.print("s");
		} else if (gap_time > 2000){ // ab 2 sekunden
			lcd.print(gap_time/1000);
			lcd.print(",");
			lcd.print(gap_time/100);
			lcd.print("s");
		}   else {
			lcd.print(gap_time);
			lcd.print("ms");
		}

	}else if (name == "Bulb_exp_time_1") {
		lcd.clear();
		lcd.print("BulbTimer");
		lcd.setCursor(0,1);

		printNumber(u_bulb_exp_time_min);
		lcd.print(":");
		printNumber(u_bulb_exp_time_s);
		lcd.print(":");
		printNumber(u_bulb_exp_time_ms);
		lcd.print(" m:s:hs");

	}else if (name == "Bulb_exp_time_min") {
		setNumber(u_bulb_exp_time_min,0,1);

	}else if (name == "Bulb_exp_time_s") {
		setNumber(u_bulb_exp_time_s,3,1);

	}else if (name == "Bulb_exp_time_ms") {
		setNumber(u_bulb_exp_time_ms,6,1);

	}else if (name == "Bulb_wait_time_1") {
		lcd.clear();
		lcd.print("waitTime");
		lcd.setCursor(0,1);
		printNumber(u_bulb_wait_time_s);
		lcd.print(" s");
		lcd.setCursor(0,1);
	}else if (name == "Bulb_wait_time_s") {
		setNumber(u_bulb_wait_time_s,0,1);

	}else if (name == "Bulb_go") {
		lcd.clear();
		lcd.print("BulbTimer");
		lcd.setCursor(0,1);
		lcd.print("Enter to proceed");

	} else {
		lcd.clear();
		lcd.noBlink();
		lcd.print(changed.to.getName());
	}
}

void menuUsed(MenuUseEvent used){
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("You used        ");
	lcd.setCursor(0,1);
	lcd.print(used.item.getName());
	delay(3000);  //delay to allow message reading
	menu.toRoot();  //back to Main
}

void buildMenu() {
//	menu sieht folgendermassen aus:
//		die Untermenus fuer die einstellungen wurden ausgelassen

//	intervalvo
//		invo_rec_time
//			invo_play_time
//				invo_fps
//					invo_go_1
//		invo_gap_time
//			invo_ctr_max
//				invo_go_2
//	bulb
//		bulb_exp_time
//			bulb_wait_time
//				bulb_go
//	combi
//	settings
//		lights
//		moduls
//		credits
//		variables
//
//		menu wird in der Reihenfolge aufgebaut.
//		freie Zeile nach jeder auswahlmoeglichkeit

	// um root immer intervalvo, fuer den start.
	menu.getRoot().addAfter(intervalvo);
	menu.getRoot().addRight(intervalvo);
	menu.getRoot().addLeft(intervalvo);
	menu.getRoot().addBefore(intervalvo);
		intervalvo.addRight(invo_rec_time_1);
		invo_rec_time_1.addRight(invo_rec_time_h);
		invo_rec_time_h.addRight(invo_rec_time_min);
				invo_rec_time_min.addRight(invo_play_time_min);
				invo_play_time_min.addRight(invo_play_time_s);
					invo_play_time_s.addRight(invo_fps_set);
						invo_fps_set.addRight(invo_go_1);
			// zurueck
				invo_play_time_min.addLeftX(invo_rec_time_h);
					invo_fps_set.addLeftX(invo_play_time_min);
						invo_go_1.addLeftX(invo_fps_set);
		invo_rec_time_1.addAfter(invo_gap_time_1);
			invo_gap_time_1.addRight(invo_gap_time_s);
			invo_gap_time_s.addRight(invo_gap_time_ms);
				invo_gap_time_ms.addRight(invo_ctr_max);
					invo_ctr_max.addRight(invo_go_2);
			//zurueck
					invo_ctr_max.addLeftX(invo_gap_time_1);
		// unendlich
			invo_gap_time_1.addAfter(invo_rec_time_1);

		//zurueck zu Intervalvo
			invo_gap_time_1.addLeft(intervalvo);
			invo_rec_time_1.addLeft(intervalvo);

		intervalvo.addAfter(bulb_1);
			bulb_1.addRight(bulb_exp_time_1);
				bulb_exp_time_1.addRight(bulb_exp_time_min);
					bulb_exp_time_min.addRight(bulb_exp_time_s);
						bulb_exp_time_s.addRight(bulb_exp_time_ms);
							bulb_exp_time_ms.addRight(bulb_wait_time_1);
								bulb_wait_time_1.addRight(bulb_wait_time_s);
									bulb_wait_time_s.addRight(bulb_go);
		// zueruck
								bulb_go.addLeftX(bulb_wait_time_1);
							bulb_wait_time_1.addLeftX(bulb_exp_time_1);

		bulb_1.addAfter(combi_1);

		combi_1.addAfter(settings_1);
			settings_1.addRight(settings_light);
			settings_light.addAfter(settings_moduls);
			settings_moduls.addAfter(settings_credits);
			settings_credits.addAfter(settings_variables);
		// unendlich
			settings_variables.addAfter(settings_light);
		//zuerueck
			settings_moduls.addLeft(settings_1);
			settings_credits.addLeft(settings_1);
			settings_variables.addLeft(settings_1);
			settings_light.addLeft(settings_1);

		// unendlich
			settings_1.addAfter(intervalvo);


}

void setup()
{
	cam_interface.pinOff(shPin1);
	cam_interface.pinOff(shPin2);
	cam_interface.pinOff(shPin3);
	cam_interface.pinOff(shPin4);
	cam_interface.pinOff(shPin5);
	cam_interface.pinOff(shPin6);

	lcd.createChar(1, letter_ae);
	lcd.createChar(2, letter_oe);
	lcd.createChar(3, load_1);
	lcd.createChar(4, load_2);
	lcd.createChar(5, load_3);

	lcd.begin(16, 2);
	pinMode(cmd_1, INPUT);
	digitalWrite(cmd_1, HIGH);
	pinMode(cmd_2, INPUT);
	digitalWrite(cmd_2, HIGH);
	pinMode(cmd_3, INPUT);
	digitalWrite(cmd_3, HIGH);
	pinMode(cmd_4, INPUT);
	digitalWrite(cmd_4, HIGH);

	//configure menu
	buildMenu();

if (!develop){
	lcd.clear();
	lcd.print(" Kamera K");
	lcd.write(1);
	lcd.print("stli ");
	lcd.setCursor(6, 1);
	lcd.print("V1.2");
	for(byte i=0; i<3; i++)
	{
		lcd.setCursor(15, 1);
		lcd.write(3);
		delay(300);
		lcd.setCursor(15, 1);
		lcd.write(4);
		delay(300);
		lcd.setCursor(15, 1);
		lcd.write(5);
		delay(300);
	}
	lcd.clear();
	lcd.print("Raphael");
	lcd.setCursor(2, 1);
	lcd.print("  and Johannes");
	delay(1000);
}
	menu.toRoot();
}

void loop()
{
	//menu(1,1,1,1);
	readButtons();  //I splitted button reading and navigation in two procedures because
	navigateMenus();  //in some situations I want to use the button for other purpose (eg. to change some settings)


}


