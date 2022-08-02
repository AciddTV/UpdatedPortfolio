package com.example.a17605096opsc7312.ui.home;

import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModelProviders;
import com.example.a17605096opsc7312.R;
import com.example.a17605096opsc7312.ui.NetworkUtil;
import com.google.android.material.snackbar.Snackbar;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

import java.io.IOException;
import java.net.URL;

public class HomeFragment extends Fragment {

    private HomeViewModel homeViewModel;
    final FirebaseDatabase firedb =FirebaseDatabase.getInstance();
    DatabaseReference ref =firedb.getReference("Users");
    String tUser;

    public View onCreateView(@NonNull LayoutInflater inflater,
            ViewGroup container, Bundle savedInstanceState) {
        homeViewModel =
                ViewModelProviders.of(this).get(HomeViewModel.class);
        View root = inflater.inflate(R.layout.fragment_home, container, false);
        final TextView textView = root.findViewById(R.id.text_home);
        homeViewModel.getText().observe(getViewLifecycleOwner(), new Observer<String>() {
            @Override
            public void onChanged(@Nullable String s) {
                textView.setText("");
            }
        });

        //__________________________________________________________________________________________Component Declerations

        final Button btnTest;
        final Button btnLog = (Button) root.findViewById(R.id.btnLogin);
        final EditText edtUser = (EditText)root.findViewById(R.id.edtUsername);
        final EditText edtPass = (EditText)root.findViewById(R.id.edtPassword);

        //__________________________________________________________________________________________
        btnLog.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(edtUser.getText().toString().equals("ChalkMagnetGT") && edtPass.getText().toString().equals("Hunter_714")){
                    Snackbar snackbar = Snackbar.make(v, "You have successfully logged in", Snackbar.LENGTH_LONG);
                    snackbar.show();
                }
                else{
                    Snackbar snackbar = Snackbar.make(v, "Username/Password is incorrect", Snackbar.LENGTH_LONG);
                    snackbar.show();
                }
            }
        });

        ref.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                String value = snapshot.getValue(String.class);
                tUser = snapshot.getValue(String.class);
            }

            @Override
            public void onCancelled(@NonNull DatabaseError error) {
                
            }
        });

        return root;
    }
}
class FetchWeather extends AsyncTask<URL, Void, String> {
    @Override
    protected String doInBackground(URL... urls){
        URL weatherURL = urls[0];
        String weatherData = null;
        try{
            weatherData = NetworkUtil.getResponseFromHttpUrl(weatherURL);
        } catch (IOException e){
            e.printStackTrace();
        }
        return weatherData;
    }
    @Override
    protected void onPostExecute(String weatherData){
        if(weatherData != null){

        }
        super.onPostExecute(weatherData);
    }
}