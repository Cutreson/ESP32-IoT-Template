package cutreson.com.android_client;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import org.json.JSONException;
import org.json.JSONObject;

import com.github.nkzawa.emitter.Emitter;
import com.github.nkzawa.socketio.client.IO;
import com.github.nkzawa.socketio.client.Socket;

import java.net.URISyntaxException;

public class MainActivity extends AppCompatActivity {
    private Socket mSocket;
    {
        try {
            mSocket = IO.socket("https://esp32-iot-template.herokuapp.com");
        } catch (URISyntaxException e) {}
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mSocket.connect();

        final Button btnOn = findViewById(R.id.btnOn);
        final Button btnOff = findViewById(R.id.btnOff);
        final TextView txtStatus = findViewById(R.id.txtStatus);

        btnOn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                String jsonString = "{name: \"Android\", address: \"led\", value: \"on\"}";
                try {
                    JSONObject jsonData = new JSONObject(jsonString);
                    mSocket.emit("browser-send-data",jsonData);
                } catch (JSONException e) {

                }
            }
            });
        btnOff.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                String jsonString = "{name: \"Android\", address: \"led\", value: \"off\"}";
                try {
                    JSONObject jsonData = new JSONObject(jsonString);
                    mSocket.emit("browser-send-data",jsonData);
                } catch (JSONException e) {

                }
            }
        });

        mSocket.on("server-send-browser", new Emitter.Listener() {
            @Override
            public void call(Object... args) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        JSONObject data = (JSONObject) args[0];
                        String value = data.optString("value");
                        if(value.equals("on")){
                            txtStatus.setText("ON");
                        }
                        if(value.equals("off")){
                            txtStatus.setText("OFF");
                        }
                    }
                });
            }
        });
    }
}