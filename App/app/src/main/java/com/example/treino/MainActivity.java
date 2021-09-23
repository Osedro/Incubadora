package com.example.treino;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class MainActivity extends AppCompatActivity {

    private Socket socket;
    private PrintWriter output;
    private BufferedReader input;
    private String ip;
    private String tempo;
    private Switch switchBomba, switchLuz;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        switchBomba = findViewById(R.id.switchBomba);
        switchLuz = findViewById(R.id.switchLuz);

        switchBomba.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                EditText ip_view = findViewById(R.id.editTextIP);
                ip = "192.168.0."+ip_view.getText().toString();

                if(b){
                    Thread thread = new Thread(new Runnable() {
                        @Override
                        public void run() {
                            output.print("b1");
                            output.flush();
                        }
                    });
                    thread.start();
                }else{
                    Thread thread = new Thread(new Runnable() {
                        @Override
                        public void run() {
                            output.print("b0");
                            output.flush();
                        }
                    });
                    thread.start();
                }
            }
        });

        switchLuz.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                EditText ip_view = findViewById(R.id.editTextIP);
                ip = "192.168.0."+ip_view.getText().toString();
                System.out.println(b);
                if(b){
                    Thread thread = new Thread(new Runnable() {
                        @Override
                        public void run() {
                            output.print("l1");
                            output.flush();
                        }
                    });
                    thread.start();
                }else{
                    Thread thread = new Thread(new Runnable() {
                        @Override
                        public void run() {
                            output.print("l0");
                            output.flush();
                        }
                    });
                    thread.start();
                }
            }
        });

    }

    public void conectar(View view){
        EditText ip_view = findViewById(R.id.editTextIP);
        ip = "192.168.0."+ip_view.getText().toString();

        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    if(socket == null || !socket.isConnected()){
                        socket = new Socket(ip, 8080);
                        output = new PrintWriter(socket.getOutputStream());
                    }

                } catch (IOException e) {
                    e.printStackTrace();
                }


            }
        });
        thread.start();
    }

    public void luzTempo(View view){
        EditText tempo_view = findViewById(R.id.editTextTempo);
        tempo = (tempo_view.getText()).toString();
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                output.print("L"+tempo);
                output.flush();
            }
        });
        thread.start();
    }

    public void bombaTempo(View view){
        EditText tempo_view = findViewById(R.id.editTextTempo);
        tempo = (tempo_view.getText()).toString();
        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                output.print("B"+tempo);
                output.flush();
            }
        });
        thread.start();
    }
}