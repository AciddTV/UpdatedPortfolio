package com.example.a17605096opsc7312.ui;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;
import android.view.View;

import java.io.File;

public class ProcessImageUtil {
        private ProcessImageUtil(){}
        public static Bitmap takeScreenshot(View view){
            View screenView = view.getRootView();
            screenView.setDrawingCacheEnabled(true);
            Bitmap bitmap = Bitmap.createBitmap(screenView.getDrawingCache());
            screenView.setDrawingCacheEnabled(false);
            return bitmap;
        }

        public static void storeScreenshot(Context context, Bitmap bitmap, String filename){
            final File directory = context.getExternalFilesDir(null);
            if(!directory.exists()){
                boolean isCreated = directory.mkdirs();
                Log.d("MakingDir", "Created: " + isCreated);
            }
            File captureImage = new File(directory, filename+".PNG");
        }

}
