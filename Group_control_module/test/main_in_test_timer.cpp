#include <Arduino.h>
#include <Time_control.h>

dtc::Grow_timer prd1;
dtc::Grow_timer prd2;
dtc::Time_channel ch1;
dtc::Time_channel ch2;
dtc::Time_channel ch3;
dtc::Time_channel ch4;
dtc::Time_channel ch5;

void print_ch(const dtc::Time_channel &channel) {
    Serial.print("CH: {");
    Serial.print("d_on=");
    Serial.print(channel.get_duration_on());
    Serial.print("  d_off=");
    Serial.print(channel.get_duration_off());
    Serial.println("}");
}
void print_ch(const dtc::Grow_timer &timer) {
    Serial.print("GT: {");
    Serial.print(" (");
    Serial.print((uint32_t)&timer);
    Serial.println(")");

    Serial.print("     DT={");
    Serial.print(timer.get_start_hours());
    Serial.print(":");
    Serial.print(timer.get_start_minutes());
    Serial.print(":");
    Serial.print(timer.get_start_seconds());
    Serial.println("}");

    dtc::Grow_timer::Time time = timer.get_end_time();
    Serial.print("     NT={");
    Serial.print(time.hour);
    Serial.print(":");
    Serial.print(time.min);
    Serial.print(":");
    Serial.print(time.sec);
    Serial.println("}");

    Serial.print("     CH[");
    Serial.print(timer.get_amt_channel());
    if(timer.get_amt_channel()) {
        Serial.println("]={");
        for(int i = 0; i < timer.get_amt_channel(); ++i) {
            Serial.print("         ");
            print_ch(*timer.get_channel(i));
        }
        Serial.println("     }");
    }
    else
        Serial.println("]={ }");

    Serial.println("}");
}

uint8_t cr_num = 71;

dtc::Grow_timer* connecter=nullptr;
dtc::Time_channel creater_channel() {
    dtc::Time_channel chl;
    return chl;
}

dtc::Time_channel* connect=nullptr;
dtc::Grow_timer creater_timer() {
    dtc::Grow_timer timer;
    if(connect != nullptr)
        timer.bind_channel(*connect);
    return timer;
}

void setup() {
    Serial.begin(115200);
    while(!Serial) {}

    {
        // prd1.set_id(101); 
        prd1.set_start_time({ 6, 30, 15}); prd1.set_end_time({16, 22, 02});
        // prd2.set_id(102); 
        prd2.set_start_time({18, 45, 55}); prd2.set_end_time({ 4, 44, 04});

        // ch1.set_id(1); 
        ch1.set_duration_on(100); ch1.set_duration_off(600);
        // ch2.set_id(2); 
        ch2.set_duration_on(200); ch2.set_duration_off(700);
        // ch3.set_id(3); 
        ch3.set_duration_on(300); ch3.set_duration_off(800);
        // ch4.set_id(4); 
        ch4.set_duration_on(400); ch4.set_duration_off(900);
        // ch5.set_id(5); 
        ch5.set_duration_on(500); ch5.set_duration_off(999);
    }
    delay(1000);
    
    Serial.println("Start!");

    {
        Serial.println("Channels:");
        print_ch(ch1);
        print_ch(ch2);
        print_ch(ch3);
        print_ch(ch4);
        print_ch(ch5);
        Serial.println("Periods:");
        print_ch(prd1);
        print_ch(prd2);
    }



    if(1) {
        Serial.println("\nBind!\n");
        prd1.bind_channel(ch1);
        prd1.bind_channel(ch2);
        prd1.bind_channel(ch3);
        prd2.bind_channel(ch4);
        prd2.bind_channel(ch5);

        Serial.println("Channels:");
        print_ch(ch1);
        print_ch(ch2);
        print_ch(ch3);
        print_ch(ch4);
        print_ch(ch5);
        Serial.println("Periods:");
        print_ch(prd1);
        print_ch(prd2);
    }

    if(1) {
        Serial.println("\nUnbind!\n");
        prd1.unbind_channel(ch2);
        prd2.unbind_channel(ch5);

        Serial.println("Channels:");
        print_ch(ch1);
        print_ch(ch2);
        print_ch(ch3);
        print_ch(ch4);
        print_ch(ch5);
        Serial.println("Periods:");
        print_ch(prd1);
        print_ch(prd2);
    }

    if(0) {
        Serial.println("\nError bind!");
        if(prd1.bind_channel(ch4))
            Serial.println("Error!");

        Serial.println("Channels:");
        print_ch(ch1);
        print_ch(ch2);
        print_ch(ch3);
        print_ch(ch4);
        print_ch(ch5);
        Serial.println("Periods:");
        print_ch(prd1);
        print_ch(prd2);
    }

    if(0) {
        Serial.println("\n\nBlock!\n");
        {        
            dtc::Grow_timer prd3;
            // prd3.set_id(103);
            prd3.set_start_time({ 1, 2, 3}); prd3.set_end_time({2, 3, 4});
            prd3.bind_channel(ch2);


            Serial.println("Channel:");
            print_ch(ch2);
            Serial.println("Periods:");
            print_ch(prd3);
        }

        Serial.println("Channel:");
        print_ch(ch2);

        Serial.println();
        {        
            dtc::Time_channel ch6;
            // ch6.set_id(6);
            ch6.set_duration_on(123); ch6.set_duration_off(234);

            Serial.println("Channel:");
            print_ch(ch6);

            prd1.bind_channel(ch6);


            Serial.println("Channel:");
            print_ch(ch6);
            Serial.println("Periods:");
            print_ch(prd1);
        }

        Serial.println("Periods:");
        print_ch(prd1);
    }

    if(1) {
        Serial.println("\n\nCreater!\n");


        dtc::Grow_timer prd_c1 = creater_timer();
        Serial.println("Period:");
        print_ch(prd_c1);

        dtc::Time_channel ch_c1 = creater_channel();
        Serial.println("Channel:");
        print_ch(ch_c1);

        connecter = &prd_c1;
        connect = &ch_c1;

        dtc::Grow_timer prd_c2 = creater_timer();
        dtc::Time_channel ch_c2 = creater_channel();

        Serial.println("\nChannel:");
        print_ch(ch_c1);
        print_ch(ch_c2);
        Serial.println("Period:");
        print_ch(prd_c1);
        print_ch(prd_c2);
    }

}

void loop() {
    // delay(1000);
    // Serial.print("Pin4 = ");
    // Serial.println(digitalRead(GPIO_NUM_4));
    // Serial.print("Pin5 = ");
    // Serial.println(digitalRead(GPIO_NUM_5));
}



#if defined ( ASDFG )
#if defined ( ASDFG )

#include <Arduino.h>

#define PIN_LED GPIO_NUM_25

void setup() {
    while(1);
    Serial.begin(115200);
    while(!Serial) {}
    delay(1000);
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
    Serial.print("Start!\nAmount LED blinks = ");
    Serial.println(AMT_LED_BLINKS);
}
void loop() {
    for(int i = 0; i < AMT_LED_BLINKS; ++i) {
        digitalWrite(PIN_LED, HIGH);
        delay(250);
        digitalWrite(PIN_LED, LOW);
        delay(250);
    }
    delay(5000);
}

#endif


#else 

#define EMPTY
#ifdef EMPTY

#if defined ( ASDFG )
#include <Arduino.h>


void setup() {
    Serial.begin(115200);
    while(!Serial) {}
    delay(500);
    Serial.println("Start!");
    // put your setup code here, to run once:
}
void loop() {
    // put your main code here, to run repeatedly:
    static unsigned long timepoint = millis();
	if (millis() - timepoint > 1000U) { // ожидание 1с
        // ожидающие операции
        Serial.print(".");
		timepoint = millis();
    }
}

#endif


#endif

#ifndef EMPTY
#include <Arduino.h>
void setup() {
    Serial.begin(115200);
    while(!Serial) {}
    delay(5000);
    pinMode(GPIO_NUM_4, INPUT);
    pinMode(GPIO_NUM_5, INPUT);
}
void my_sleep() { while(Serial.read()!='\n')delay(100); }
void loop() {
    int p4, p5;
    p4 = digitalRead(GPIO_NUM_4);
    p5 = analogRead(GPIO_NUM_5);
    Serial.print("Input at pin 4 = ");
    Serial.println(p4);
    Serial.print("Input at pin 5 = ");
    Serial.println(p5);

    delay(1000);

    while(Serial.read()!='\n')delay(500);


    static unsigned long timepoint = millis();
	if (millis() - timepoint > 1000U) { // ожидание 1с
        // ожидающие операции
		timepoint = millis();
    }
    // операции вне ожидания
}
#endif

#endif