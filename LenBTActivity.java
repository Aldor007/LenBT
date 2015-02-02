package len.robot;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Iterator;
import java.util.Set;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class LenBTActivity extends Activity {
    /** Called when the activity is first created. */
	  // debug
	final String tag = "LenBT";
	final String ROBOTNAME = "KAmodBT demo";
	final boolean D = true;

    private int kierunek = 0;
    //bt
    private BluetoothAdapter btInterface; //interfejs bt
	private Set<BluetoothDevice> pairedDevices; //sparowane urzadzenia
	private BluetoothSocket socket; //soket dla rfcomm
	private OutputStream os = null; //strumien wyjsciow
	private boolean btConnected = false; //czy poloczne

	// broadcast receiver to handle bt events
		private BroadcastReceiver btMonitor = null;

    // Layout Views

    private ImageButton[] butKierunki;
    private Button butConnect;
    private Button butDisconnect;
    private TextView info;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if(D) Log.i(tag,"OnCreate");
        setContentView(R.layout.main);
        setupUI();
        setupBTMonitor();
    }
    @Override
public void onResume()
{
	super.onResume();
	Log.i(tag,"onResume");
	//rejestracja sluchacza dla poloczenia bt
	registerReceiver(btMonitor,new IntentFilter("android.bluetooth.device.action.ACL_CONNECTED"));
	registerReceiver(btMonitor,new IntentFilter("android.bluetooth.device.action.ACL_DISCONNECTED"));
}
@Override
public void onPause()
{
	super.onPause();
	Log.i(tag,"onPause");
	//wyrejestrowanie sluchacz
	unregisterReceiver(btMonitor);
}
    private void setupUI()
    {//dla wygladu
    	 butKierunki = new ImageButton[5];
         butKierunki[0] = (ImageButton)findViewById(R.id.n2);
         butKierunki[1] = (ImageButton)findViewById(R.id.n8);
         butKierunki[2] = (ImageButton)findViewById(R.id.n4);
         butKierunki[3] = (ImageButton)findViewById(R.id.n6);
         butKierunki[4] = (ImageButton)findViewById(R.id.n5);
         butKierunki[0].setOnClickListener(new View.OnClickListener() {

 			public void onClick(View v) {
 				kierunek  = 2;
 				updateMotors();

 			}
 		});
         butKierunki[1].setOnClickListener(new View.OnClickListener() {

     			public void onClick(View v) {
     				kierunek  = 8;
     				updateMotors();
     			}
     		});
         butKierunki[2].setOnClickListener(new View.OnClickListener() {

     			public void onClick(View v) {
     				kierunek  = 4;
     				updateMotors();
     			}
     		});
         butKierunki[3].setOnClickListener(new View.OnClickListener() {

 			public void onClick(View v) {
 				kierunek  = 6;
 				updateMotors();
 			}
 		});
         butKierunki[4].setOnClickListener(new View.OnClickListener() {

  			public void onClick(View v) {
  				kierunek  = 5;
  				updateMotors();
  			}
  		});


         butConnect = (Button)findViewById(R.id.connect);
         butDisconnect = (Button)findViewById(R.id.disconnect);

         info  = (TextView)findViewById(R.id.textInfo);
    	onDisconnectsetUI();
    }

    private void updateMotors() {
    	try {
    		//wyslanie informacje do robota
    		byte tmp ='5';
    		switch(kierunek)
    		{
    		case 2: tmp = '2'; break;
    		case 8: tmp = '8'; break;
    		case 6: tmp = '4'; break;
    		case 4: tmp = '6'; break;
    		case 5: tmp = '5'; break;
    		}
    		os.write(tmp);

    	} catch (Exception e) {
    		Log.e(tag,"updateMotors error::" + e.getMessage());
    	}
    }

    private void onDisconnectsetUI()
    {
    	//zmiana wygldu dla braku polaczenia
    	 for(int i=0;i<butKierunki.length;i++)
             butKierunki[i].setVisibility(View.GONE);
             butConnect.setVisibility(View.VISIBLE);
             butDisconnect.setVisibility(View.GONE);

    }
    private void onConnectsetUI()
    {
    	//zmiana wygladu dla polczenia
    	 for(int i=0;i<butKierunki.length;i++)
             butKierunki[i].setVisibility(View.VISIBLE);
             butConnect.setVisibility(View.GONE);
             butDisconnect.setVisibility(View.VISIBLE);

    }
    private void setupBTMonitor() {
    	// tworzenie sluchacza dla zdarzen bt
        btMonitor = new BroadcastReceiver() {
        	@Override
        	public void onReceive(Context context,Intent intent) {
        		if (intent.getAction().equals("android.bluetooth.device.action.ACL_CONNECTED")) {
        			handleConnected();
        		}
        		if (intent.getAction().equals("android.bluetooth.device.action.ACL_DISCONNECTED")) {
        			handleDisconnected();
        		}

        	}
        };

    }
    private void handleConnected() {
		try {
			//mamy poloczenie
			if(D) Log.i(tag,"Open outputStream");
			os = socket.getOutputStream();
			btConnected = true;
			onConnectsetUI();

		} catch (Exception e) {

			os = null;
			disconnectFromRobot(null);
			if(D) Log.e(tag,"Error handleConnection");
		}

    }
    private void handleDisconnected() {
    	//rozloczony
		btConnected = false;
		onDisconnectsetUI();

		info.setText("Sensory wylaczone.  Prosze podlaczyc robota aby kontynuowac.");
    }
    private void connectToRobot(BluetoothDevice bd)
   	{
    	//towrzenie poloczenia rfcomm do robota
   		try
   		{
   			if(D) Log.i(tag,"RFCOMM to"+bd.getName());
   			socket = bd.createRfcommSocketToServiceRecord(java.util.UUID.fromString("00001101-0000-1000-8000-00805F9B34FB"));
   			socket.connect();
   		}
   		catch (Exception e)
   		{
   			Log.e(tag,"Error interacting with remote device [" + e.getMessage() + "]");
   		}
   	}

   	public void disconnectFromRobot(View v)
   	{
   		//zamkniecie poloczenia
   		try
   		{
   			if(D) Log.i(tag,"Attempting to break BT connection");
   			socket.close();
   		}
   		catch (Exception e)
   		{
   			Log.e(tag,"Error in DoDisconnect [" + e.getMessage() + "]");
   		}
   	}

    public void findRobot(View v)
    {
    	try
		{
    		//szukanie robota w sparowanych urzadzeniach
    		btInterface = BluetoothAdapter.getDefaultAdapter();
    		if(D) Log.i(tag,"Local BT Interface name is [" + btInterface.getName() + "]");
    		pairedDevices = btInterface.getBondedDevices();
    		if(D) Log.i(tag,"Found [" + pairedDevices.size() + "] devices.");
    		Iterator<BluetoothDevice> it = pairedDevices.iterator();
    		while (it.hasNext())
    		{
    			BluetoothDevice bd = it.next();
    			if(D) Log.i(tag,"Name of peer is [" + bd.getName() + "]");
    			if (bd.getName().equalsIgnoreCase(ROBOTNAME)) {
    				if(D)
    					{
    					Log.i(tag,"Found Robot!");
    					Log.i(tag,bd.getAddress());
    				 	Log.i(tag,bd.getBluetoothClass().toString());
    					}
    				 connectToRobot(bd);

    				return;
    			}
    		}
		}
		catch (Exception e)
		{
			Log.e(tag,"Failed in findRobot() " + e.getMessage());
		}


    }

}
