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

import net.rim.device.api.ui.Field;
import net.rim.device.api.ui.Graphics;
import net.rim.device.api.ui.TouchEvent;

public class ColorMixer extends Field {
    private static final int[] colors = new int[] { 0xff0000, 0xff00, 0xff };
    private int barGap = 5;

    private int barHeight = 64;
    private int color;
    private MyScreen owner;

    private double r, g, b;

    public ColorMixer(MyScreen owner) {
        this.owner = owner;
    }

    protected void layout(int width, int height) {
        setExtent(640, barHeight * 3 + barGap * 2);
    }

    protected void paint(Graphics graphics) {
        int barWidth = getWidth() - barHeight * 3 - barGap * 3;
        double[] values = new double[] { r, g, b };

        // Draw color sliders
        for (int i = 0; i < 3; ++i) {
            int top = i * (barHeight + barGap);
            int toDraw = (int) (values[i] * barWidth + 0.5);
            graphics.setColor(0);
            graphics.fillRect(toDraw, top, barWidth - toDraw, barHeight);

            graphics.setColor(colors[i]);
            graphics.fillRect(0, top, toDraw, barHeight);

            graphics.setColor(0x7f7f7f);
            graphics.drawRect(0, top, barWidth, barHeight);
        }

        // Draw mixed down color
        int boxWidth = barHeight * 3 + barGap * 2;
        graphics.setColor(color);
        graphics.fillRect(getWidth() - 1 - boxWidth, 0, boxWidth, boxWidth);

        graphics.setColor(0x7f7f7f);
        graphics.drawRect(getWidth() - 1 - boxWidth, 0, boxWidth, boxWidth);
    }

    public void setColor(int color) {
        if (color != this.color) {
            this.color = color;
            r = ((color >> 16) & 0xff) / 255.0;
            g = ((color >> 8) & 0xff) / 255.0;
            b = (color & 0xff) / 255.0;
            invalidate();
        }
    }

    private void setPart(int which, double value) {
        double source = which == 0 ? r : which == 1 ? g : b;
        if (value == source) {
            return;
        }
        int bv = (int) (value * 255 + 0.5);
        bv &= 0xff;
        if (which == 0) {
            r = value;
            color &= 0xffff;
            color |= bv << 16;
        } else if (which == 1) {
            g = value;
            color &= 0xff00ff;
            color |= bv << 8;
        } else {
            b = value;
            color &= 0xffff00;
            color |= bv;
        }
        owner.setColor(color);
        invalidate();
    }

    private int touchTarget = -1;

    protected boolean touchEvent(TouchEvent message) {
        int event = message.getEvent();

        int x = message.getX(1);
        int y = message.getY(1);

        int barWidth = getWidth() - barHeight * 3 - barGap * 3;

        if (event == TouchEvent.UP) {
            touchTarget = -1;
            return false;
        } else if (event == TouchEvent.DOWN) {
            if (y < -5 || y > getWidth() + 5) {
                return false;
            }

            if (x <= barWidth) {
                for (int i = 0; i < 3; ++i) {
                    if (y < 0) {
                        break;
                    }
                    if (y < barHeight) {
                        touchTarget = i;
                        break;
                    }
                    y -= barHeight;
                    y -= barGap;
                }
            } else {
                return false;
            }
        }

        if (touchTarget == -1) {
            return false;
        }

        double newValue = 1.0 * x / barWidth;
        if (newValue < 0) {
            newValue = 0;
        }
        if (newValue > 1) {
            newValue = 1;
        }

        setPart(touchTarget, newValue);

        return true;
    }
}
