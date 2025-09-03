/**  
   Copyright (c) 2011, 2012, 2013 Research In Motion Limited.
  
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
  
   http://www.apache.org/licenses/LICENSE-2.0
  
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**/
package mypackage;

import net.rim.device.api.io.nfc.NFCException;
import net.rim.device.api.io.nfc.emulation.VirtualISO14443Part4TargetCallback;
import net.rim.device.api.io.nfc.emulation.VirtualISO14443Part4TypeATarget;
import net.rim.device.api.ui.Field;
import net.rim.device.api.ui.FieldChangeListener;
import net.rim.device.api.ui.component.CheckboxField;
import net.rim.device.api.ui.component.LabelField;
import net.rim.device.api.ui.container.MainScreen;
import net.rim.device.api.ui.decor.BackgroundFactory;

/**
 * A class extending the MainScreen class, which provides default standard
 * behavior for BlackBerry GUI applications.
 */
public final class MyScreen extends MainScreen implements
        VirtualISO14443Part4TargetCallback, FieldChangeListener {

    private int color = 0x7f7f7f;

    private boolean colorQueued = false;
    private CheckboxField colorQueueLocked;
    private VirtualISO14443Part4TypeATarget emulated;
    private ColorMixer mixer;

    private LabelField status;

    /**
     * Creates a new MyScreen object
     */
    public MyScreen() {
        // Set the displayed title of the screen
        setTitle("Colour Control Setter");
        setBackground(BackgroundFactory.createSolidBackground(0x3f3f3f));

        add(new LabelField(""));
        add(colorQueueLocked = new CheckboxField("Lock Color", false));
        colorQueueLocked.setChangeListener(this);

        add(mixer = new ColorMixer(this));
        mixer.setColor(0x7f7f7f);

        add(status = new LabelField(""));

        try {
            emulated = new VirtualISO14443Part4TypeATarget(this, "10451045",
                    null);
            log("created");
            emulated.startEmulation();
            log("emulating");
        } catch (NFCException e) {
            log("Failed to listen");
            status.setText(e.getClass().getName() + " " + e.getMessage());
            e.printStackTrace();
        }
    }

    // private TextField textField;

    public void fieldChanged(Field field, int context) {
        if (colorQueueLocked.getChecked()) {
            setColor(color);
        } else {
            colorQueued = false;
            status.setText("");
        }
    }

    private boolean isColorQueueLocked() {
        return colorQueueLocked.getChecked();
    }

    protected void log(String message) {
        System.out.println("SUPERDUPER: " + message);
        // textField.setText(textField.getText() + "\n" + message);
    }

    public void onVirtualTargetEvent(int targetEvent) {
        log("Target event: " + targetEvent);
    }

    public byte[] processCommand(byte[] command) {
        byte[] toReturn = processCommandInternal(command);
        String message = "[" + toString(command) + "," + toString(toReturn)
                + "]";
        // status.setText(message);
        // log(message);
        return toReturn;
    }

    private byte[] processCommandInternal(byte[] command) {
        if (command == null) {
            return new byte[0];
        }
        int messageType = command[0] & 0xff;
        switch (messageType) {
        case 1:
            if (command.length == 4) {
                int colorIn = (command[1] & 0xff) << 16;
                colorIn |= (command[2] & 0xff) << 8;
                colorIn |= (command[3] & 0xff);

                if (colorIn != color) {
                    if (colorQueued) {

                        byte[] toReturn = new byte[4];
                        toReturn[0] = 1;
                        toReturn[1] = (byte) (color >> 16);
                        toReturn[2] = (byte) (color >> 8);
                        toReturn[3] = (byte) color;

                        if (!isColorQueueLocked()) {
                            colorQueued = false;
                            status.setText("");
                        }

                        return toReturn;
                    } else {
                        this.color = colorIn;
                        mixer.setColor(colorIn);
                    }
                }
            }
            break;
        }
        return new byte[] { 0 };
    }

    public void setColor(int color) {
        colorQueued = true;
        this.color = color;
        status.setText("#" + toHex(color));
    }

    private String toHex(int i) {
        String toReturn = "000000" + Integer.toString(i, 16);
        return toReturn.substring(toReturn.length() - 6);
    }

    private String toString(byte[] command) {
        String toReturn = "";
        for (int i = 0; i < command.length; ++i) {
            toReturn += Integer.toString(command[i] & 0xff, 16) + " ";
        }
        return toReturn;
    }
}
