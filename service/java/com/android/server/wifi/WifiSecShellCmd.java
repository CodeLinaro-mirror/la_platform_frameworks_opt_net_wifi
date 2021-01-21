/* Copyright (c) 2021 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.android.server.wifi;

import android.net.wifi.WifiManager;
import android.net.ConnectivityManager;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.hotspot2.pps.Credential;
import android.net.wifi.hotspot2.pps.HomeSp;
import android.net.wifi.hotspot2.PasspointConfiguration;

import java.io.*;
import java.io.PrintWriter;
import java.util.List;
import java.util.ArrayList;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateFactory;
import com.android.internal.util.AsyncChannel;
import android.text.TextUtils;
import android.os.Process;
import android.os.Message;
import android.content.Context;

import android.net.NetworkRequest;
import static android.net.wifi.WifiManager.STA_PRIMARY;
import static android.net.wifi.WifiManager.STA_SECONDARY;
import android.net.NetworkCapabilities;
/**
 * Runs shell command for secondary station.
 *
 * To run 'adb shell cmd wifi sec [args]'
 *
 * API of this class is invoked by WifiShellCommand.
 */
public class WifiSecShellCmd {

    private final ConnectivityManager mConnectivityManager;
    private final WifiInjector mWifiInjector;
    private final ActiveModeWarden mActiveModeWarden;
    private final Context mContext;
    private final int mStaId;
    private WifiManager mWifiManager;
    private static HomeSp mHomeSp = new HomeSp();
    private static Credential mCredential = new Credential();
    private static Credential.UserCredential mUserCredential = new Credential.UserCredential();
    private static PasspointConfiguration mPasspointConfiguration = new PasspointConfiguration();;

    WifiSecShellCmd(WifiInjector injector) {
        mWifiInjector = injector;
        mActiveModeWarden = injector.getActiveModeWarden();
        mContext = injector.getContext();
        mStaId = STA_SECONDARY;
        mConnectivityManager = mContext.getSystemService(ConnectivityManager.class);
        mWifiManager = null;
    }

    private WifiManager getManager() {
        if (mWifiManager == null)
            mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        return mWifiManager;
    }

/* Lets do it through WifiManager */

    public void enable() {
       mConnectivityManager.requestNetwork(new NetworkRequest.Builder()
                            .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                            .addCapability(NetworkCapabilities.NET_CAPABILITY_OEM_PAID)
                            .build(),
                  new ConnectivityManager.NetworkCallback());
       getManager().setWifiEnabled(mStaId, true);
    }

    public void disable() {
        getManager().setWifiEnabled(mStaId, false);
    }

    public int addNetwork(String ssid, String password) {
        return getManager().addNetwork(createLocalConfig(ssid, password));
    }

    public int updateNetwork(String ssid, String password) {
        return getManager().updateNetwork(createLocalConfig(ssid, password));
    }

    public void saveNetwork(String ssid, String password) {
        getManager().save(createLocalConfig(ssid, password), null);
    }

    public boolean removeNetwork(int netId) {
        return getManager().removeNetwork(netId);
    }


    public void forgetNetwork(int netId) {
        getManager().forget(netId, null);
    }

    public void connectNetwork(String ssid, String password) {
        getManager().connect(createLocalConfig(ssid, password), null);
    }

    public void connectNetwork(int netId) {
        getManager().connect(netId, null);
    }

    public boolean enableNetwork(int netId) {
        return getManager().enableNetwork(netId, false /*disable others*/);
    }

    public void disableNetwork(int netId) {
        getManager().disable(netId, null);
    }

    public boolean disconnect() {
        return getManager().disconnect(mStaId);
    }

    public void listNetwork(PrintWriter pw) {
        List<WifiConfiguration> configs = null;

        configs = getManager().getConfiguredNetworks(mStaId);
        if (configs != null)
            pw.println("Total configurations: " + configs.size());

        for (WifiConfiguration config : configs) {
            pw.println(" [" + getStaIdString(config) + "] netId: " + config.networkId
                       + " SSID: " + config.SSID + " configkey: " + config.getKey());
        }
    }

    public void status(PrintWriter pw) {
        pw.println("WifiInfo=" + getManager().getConnectionInfo(mStaId));
    }

    public void setHomeSp(String fqdn, String friendlyName, long[] OSIs) {
        mHomeSp.setFqdn(fqdn);
        mHomeSp.setFriendlyName(friendlyName);
        mHomeSp.setRoamingConsortiumOis(OSIs);
    }

    public HomeSp getHomeSp(PrintWriter pw) {
        pw.println("HomeSp is: " + mHomeSp);
        return mHomeSp;
    }

    public void setUserCredential(String username, String password, boolean machineManaged, int eap_type, String innerMethod) {
        mUserCredential.setUsername(username);
        mUserCredential.setPassword(password);
        mUserCredential.setMachineManaged(machineManaged);
        mUserCredential.setEapType(eap_type);
        mUserCredential.setNonEapInnerMethod(innerMethod);
    }

    public Credential.UserCredential getUserCredential(PrintWriter pw) {
        pw.println("UserCredential is: " + mUserCredential);
        return mUserCredential;
    }

    private ArrayList<String> readCaFilesFromPath(String path, PrintWriter pw) {
        ArrayList<String> caFiles = new ArrayList<String>();
        int i = 0;

        File[] filesList = new File(path).listFiles();
        for (File file : filesList) {
            if (file.getName().endsWith(".pem")) {
                String fileInfo = readFile(file.getAbsolutePath(), pw);
                if (!fileInfo.equals("")) {
                    caFiles.add(fileInfo);
                }
            }
        }
        return caFiles;
    }

    private String readFile(String path, PrintWriter pw) {
        StringBuilder buffer = new StringBuilder();
        int index = 0;
        try {
            File filename = new File(path);
            InputStreamReader in = new InputStreamReader(new FileInputStream(filename));
            BufferedReader reader = new BufferedReader(in);
            String line = "";
            while((line = reader.readLine()) != null) {
                if (!line.equals("-----BEGIN CERTIFICATE-----") && index == 0) {
                    pw.println("certificate's format is not match, we can't transfer this format certificate. This certificate begin with: " + line + ", file's path is: " + path);
                    break;
                }
                buffer.append(line + '\n');
                index++;
            }
        } catch (Exception e) {
            pw.println(e);
        }
        return buffer.toString();
    }

    private String transferPathFormat(String path) {
        String[] pathes = path.split("\\/");
        StringBuffer dir = new StringBuffer();

        for (int index = 0; index < pathes.length; index++) {
            dir.append(pathes[index]);
            if (index < pathes.length - 1) {
                dir.append(File.separator);
            }
        }
        return dir.toString();
    }

    public X509Certificate[] loadCertificates(String path, PrintWriter pw) {
        ArrayList<X509Certificate> certificateList = new ArrayList<X509Certificate>();

        /* transfer path format to linux readable format. */
        String dir = transferPathFormat(path);

        /* read all certificates from the path. */
        ArrayList<String> certs = readCaFilesFromPath(dir, pw);

        try {
            for (String cert : certs) {
                CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
                ByteArrayInputStream bytes = new ByteArrayInputStream(cert.getBytes());
                certificateList.add((X509Certificate) certFactory.generateCertificate(bytes));
            }
        } catch (Exception e) {
            pw.println(e);
        }

        X509Certificate[] certificates = new X509Certificate[certificateList.size()];
        for (int index = 0; index < certificateList.size(); index++) {
            certificates[index] = certificateList.get(index);
        }
        return certificates;
    }

    public void setCredential(String realm, Credential.UserCredential uc, X509Certificate[] cas) {
        mCredential.setRealm(realm);
        mCredential.setUserCredential(uc);
        mCredential.setCaCertificates(cas);
    }

    public Credential getCredential(PrintWriter pw) {
        pw.println("Credential is: " + mCredential);
        return mCredential;
    }

    public void setPasspointConfiguration(HomeSp sp, Credential cred) {
        mPasspointConfiguration.setHomeSp(mHomeSp);
        mPasspointConfiguration.setCredential(mCredential);
        getManager().addOrUpdatePasspointConfiguration(mPasspointConfiguration, STA_SECONDARY);
    }

    public List<PasspointConfiguration> getPasspointConfigurations() {
        return getManager().getPasspointConfigurations(STA_SECONDARY);
    }

    public void removePasspointConfiguration(String fqdn) {
        getManager().removePasspointConfiguration(fqdn, STA_SECONDARY);
    }

    // Utility API

    private WifiConfiguration createLocalConfig(String ssid, String password) {
        WifiConfiguration config = new WifiConfiguration();
        config.staId = mStaId;
        config.SSID = "\"" + ssid + "\"";

        // WPA2-PSK or OPEN network
        if (TextUtils.isEmpty(password)) {
            config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
        } else {
            config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
            config.preSharedKey = "\"" + password + "\"";
        }
        return config;
    }

    private String getStaIdString(WifiConfiguration config) {
        if (config == null)
            return "config is null";

        switch (config.staId) {
            case STA_PRIMARY:
                return "PRIMARY";
            case STA_SECONDARY:
                return "SECONDARY";
            default:
                return "UNKNOWN";
        }
    }
}
