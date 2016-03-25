/**
 * Test app for Java XTools API
 * 
 * Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
 */

package HoloToolkit.Sharing;

import java.io.*;
import java.util.concurrent.*;
import com.microsoft.holotoolkit.sharing.*;

 public class JavaTestApp {

    SharingManager _myMgr;
    final boolean _autoJoin = true;
    final int _autoJoinSessionId = 0;
    NetworkConnection _onSightConnection;
    final String _prompt = "MSLIVER>";

	MSliverSessionManagerListener _sessionMgrListener;

    MSliverSessionListener _sessionListener; 

    NetworkConnectionListener _networkListener;

    SynchronousQueue<String> _commandQueue;

    Session _currentSession;

	SyncRoot _rootObject;

	boolean shouldExit;

    private class MSliverSessionListener extends SessionListener {
        
        MSliverSessionListener() {
            super();
        }

        public void OnJoinSucceeded() {
            System.out.println("********* JOINED SESSION ******");
        }
        
        public void OnJoinFailed() {
            System.out.println("********* SESSION JOIN FAILED CALLBACK ******");
        }

		public void OnSessionDisconnected() {
			System.out.println("********* SESSION DISCONNECTED ******");
		}
    }

	private class MSliverSessionManagerListener extends SessionManagerListener {
        
        MSliverSessionManagerListener() {
            super();
        }

		public void OnCreateSucceeded(Session newSession) {
			System.out.println("********* SESSION " + newSession.GetName().GetString() + " CREATED ******");

			_currentSession = newSession;
			_sessionListener = new MSliverSessionListener();

			newSession.AddListener(_sessionListener);

			System.gc();
		}

		public void OnCreateFailed(XString reason) {
			System.out.println("********* SESSION CREATE FAILED: " + reason.GetString() + " ******");
		}

		public void OnSessionAdded(Session newSession) {
			System.out.println("********* SESSION " + newSession.GetName().GetString() + " ADDED ******");
		}

		public void OnSessionClosed(Session newSession) {
			System.out.println("********* SESSION " + newSession.GetName().GetString() + " CLOSED ******");
		}

		public void OnUserJoinedSession(Session newSession, User user) {
			System.out.println("********* USER " + user.GetName().GetString() + " JOINED SESSION " + newSession.GetName().GetString() + " ******");
		}

		public void OnUserLeftSession(Session newSession, User user) {
			System.out.println("********* USER " + user.GetName().GetString() + " LEFT SESSION " + newSession.GetName().GetString() + " ******");
		}
    }

    private class InputThread extends Thread {
        InputThread() {
			this.setDaemon(true);
            final boolean enableFairnessPolicy = true;
            _commandQueue = new SynchronousQueue<String>(enableFairnessPolicy);
        }

        public void run() {
            BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
            String input = null;

            while (true) {
                try {
                    System.out.print(_prompt);
                    input = br.readLine();
                    _commandQueue.put(input);

                } catch (IOException io) {
                    System.out.println("Error parsing input.");
                } catch (InterruptedException e) {
                    System.out.println("Interruption Exception.");
                }
            }
        }
     }

    static {
        System.loadLibrary("SharingClient");
    }


    public JavaTestApp() {
    }


	private void run() {

		System.out.println("Starting XTools Console version " + VersionInfo.getVersion());

        InputThread input = new InputThread();
        input.start();

		initializeXTools();

        while(!this.shouldExit) {
            executeCommandsInBuffer();

            doUpdates();

			try {
                 Thread.sleep(10);
             } catch (InterruptedException e) {
                 System.out.println("Thread was interrupted!");
             }
        }
    }

    private void doUpdates() {
        if (_myMgr != null) {
           _myMgr.Update();
        }
    }

    private void executeCommandsInBuffer() {
        parseCommand(getNextCommand());
    }

    private String getNextCommand() {
        return _commandQueue.poll();
    }

    private void parseCommand(String command) {
        if (command == null || command.isEmpty() || command.trim().isEmpty()) {
            return;
        }

        command = command.trim();
        
        if (command.isEmpty()) {
            return;
        }

        String lowerCommand = command.toLowerCase();

        // TODO: Better and more well-abstracted parsing logic.
        if (lowerCommand.startsWith("create")) {

            String[] args = command.split(" ");

            String name = args.length > 1 ? args[1] : "TEST SESSION";

            createSession(name);

        } else if (lowerCommand.startsWith("join")) {
            // Join

            String[] args = command.split(" ");

            String name = args.length > 1 ? args[1] : "TEST SESSION";

            joinSession(name);

        } else if (lowerCommand.startsWith("leave")) {
            // leave
            leaveSession();

        } else if (lowerCommand.startsWith("ping")) {

            String[] args = command.split(" ");

            String pingString = args.length > 1 ? args[1] : "PING STRING";

            pingSession(pingString);

        } else if (lowerCommand.startsWith("cleanup")) {
            cleanUp();

		} else if (lowerCommand.startsWith("setint")) {
			String[] args = command.split(" ");

            String value = args.length > 1 ? args[1] : "0";
            setIntValue(value);

		} else if (lowerCommand.startsWith("showint")) {
            showIntValue();

		} else if (lowerCommand.startsWith("quit")) {
            shouldExit = true;

        } else {
            System.out.println("\nCommand \"" + command + "\" not recognized!");
        }

		System.gc();
    }

    private void pingSession(String pingString) {
        if (_currentSession == null) {
            System.out.println("Session not present. Cannot ping.");
            return;
        }

        NetworkConnection sessionConnection = _currentSession.GetSessionNetworkConnection();
        NetworkOutMessage msg = sessionConnection.CreateMessage((byte)MessageID.SessionControl.swigValue());
        XString pingXString = new XString(pingString);
        msg.Write(pingXString);
        sessionConnection.Send(msg);
    }

    private void initializeXTools() {
        System.out.println("Initializing XTools...");

		ClientConfig config = new ClientConfig(ClientRole.Primary);
		config.SetServerAddress("osserver.redmond.corp.microsoft.com");

        _myMgr = SharingManager.Create(config);

        if (_myMgr != null) {
            _onSightConnection = _myMgr.GetServerConnection();
            //_rootObject = new SyncRoot(_myMgr.GetRootSyncObject());

			_sessionMgrListener = new MSliverSessionManagerListener();

			_myMgr.GetSessionManager().AddListener(_sessionMgrListener);

            final PairingListener pairingListener = new PairingListener() {
                public void PairingConnectionSucceeded() {
                    System.out.println("********* INCOMING CONNECTION SUCCEEDED *********");
                }

                public void PairingConnectionFailed(PairingResult reason) {
                    System.out.println("********* INCOMING CONNECTION FAILED *********");
                }
            };

            _networkListener = new NetworkConnectionListener()
            {
                @Override
                public void OnDisconnected(NetworkConnection connection) {
                    System.out.println("Begin pairing again");
                    _myMgr.GetPairingManager().BeginPairing(new DirectPairReceiver(), pairingListener);
                }
            };

            _myMgr.GetPairingManager().BeginPairing(new DirectPairReceiver(), pairingListener);
            _myMgr.GetPairedConnection().AddListener((byte)MessageID.StatusOnly.swigValue(), _networkListener);
        }

		System.gc();
    }

    private void createSession(String name) {
        System.out.println("Creating Session...");
        
        if (_myMgr == null) {
            System.out.println("XTools not initialized! Unable to create a session.");
            return;
        }

        XString sessionName = new XString(name);
        if (!_myMgr.GetSessionManager().CreateSession(sessionName))
		{
			System.out.println("Failed to request a new session!");
			return;
		}

		System.gc();
    }

    private void joinSession(String name) {
        if (_myMgr == null) {
            System.out.println("XTools not initialized! Unable to create a debug session.");
            return;
        }

		XString nameXString = new XString(name);

		System.out.println("Joining Session " + nameXString.GetString());

		boolean bFound = false;

		for(int i = 0; i < _myMgr.GetSessionManager().GetSessionCount(); ++i)
		{
			Session session = _myMgr.GetSessionManager().GetSession(i);
			if (session.GetName().IsEqual(nameXString))
			{
				_currentSession = session;
				_sessionListener = new MSliverSessionListener();

				_currentSession.AddListener(_sessionListener);

				_currentSession.Join();

				System.out.println("Join request sent");

				bFound = true;

				break;
			}
		}

		if (!bFound)
		{
			System.out.println("Could not find session " + name);
		}

		System.gc();
    }

	private void leaveSession() {
		if (_currentSession == null)
		{
			System.out.println("Current Session is null");
		}
		else if (_currentSession.IsJoined())
		{
			_currentSession.Leave();
			
			System.out.println("Left Session " + _currentSession.GetName().GetString());

			if (_sessionListener != null) {
				_currentSession.RemoveListener(_sessionListener);
				_sessionListener = null;
			}

			_currentSession = null;
		}
		else
		{
			System.out.println("Can't leave session " + _currentSession.GetName().GetString() + ": we are not in it");
		}

		System.gc();
	}

	private void setIntValue(String newValue)
	{
		if (_rootObject != null)
		{
			_rootObject.SetIntValue(Integer.parseInt(newValue));
		}
	}

	private void showIntValue()
	{
		if (_rootObject != null)
		{
			System.out.println("Int Value: " + _rootObject.GetIntValue());
		}
	}

    private void cleanUp() {
        System.out.println("Cleaning up state...");
        
		_sessionMgrListener = null;
        _sessionListener = null;
        _currentSession = null;
        _onSightConnection = null;
        _myMgr = null;

        // TODO: Kill threads?

        System.gc();
    }

    public static void main(String[] args) {
        System.out.println("JavaTestApp Started");

        System.out.println("Working Directory = " + System.getProperty("user.dir"));

        try {
            new JavaTestApp().run();
        } catch (Exception e) {
            System.out.println("Exception Caught " + e.toString());
			e.printStackTrace();
        }

        System.gc();

        System.out.println("Done");
    }
 }

