package com.android.babylonnative.playground;

import androidx.cardview.widget.CardView;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.util.TypedValue;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;

import BabylonNative.BabylonView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.w3c.dom.Text;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;

public class MainActivity extends Activity implements BabylonView.ViewDelegate {


    private BabylonView mView;
    private CardView loginView;

    private CardView registerView;

    private TextView loginError;

    private TextView registerError;

    private LinearLayout tabView;

    private LinearLayout homeContainer;

    private TextView textOverlay;

    private Map<String, CardView> mapCards;

    private RecyclerView hotbarRecycler;

    private LinkedList<Item> hotbarItems;

    private Hotbar hotbarAdpater;

    private CardView claimTileCard;
    private LinearLayout createTileLayout;
    private LinearLayout placeTileLayout;
    private Button claimTileButton;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        FrameLayout frameLayout = (FrameLayout) findViewById(R.id.babylon_view);
        mView = new BabylonView(getApplication(), this);
        mView.setLayoutParams(new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.MATCH_PARENT));
        frameLayout.addView(mView);

        mapCards = new HashMap<String, CardView>();

        final Context context = getApplicationContext();
        ConstraintLayout constraintLayout = (ConstraintLayout) findViewById(R.id.viewContainer);
        loginView = (CardView) constraintLayout.findViewById(R.id.loginView);

        registerView = constraintLayout.findViewById(R.id.registerView);

        loginError = (TextView) constraintLayout.findViewById(R.id.loginError);

        registerError = constraintLayout.findViewById(R.id.registerError);

        tabView = findViewById(R.id.tabViewContainer);

        homeContainer = findViewById(R.id.homeContainer);

        ConstraintLayout overlayContainer = (ConstraintLayout) findViewById(R.id.overlayContainer);

        textOverlay = overlayContainer.findViewById(R.id.textOverlay);

        loginView.setVisibility(View.VISIBLE);

        hotbarItems = new LinkedList<>();
        hotbarAdpater = new Hotbar(hotbarItems, this);
        hotbarRecycler = findViewById(R.id.hotbarRecycler);
        hotbarRecycler.setAdapter(hotbarAdpater);
        hotbarRecycler.setLayoutManager(new GridAutofitLayoutManager(this, (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 130,
                context.getResources().getDisplayMetrics())));
        hotbarRecycler.setVisibility(View.VISIBLE);

        claimTileCard = tabView.findViewById(R.id.claimTileCard);
        createTileLayout = claimTileCard.findViewById(R.id.createTileLayout);
        placeTileLayout = claimTileCard.findViewById(R.id.placeTileLayout);
        claimTileButton = claimTileCard.findViewById(R.id.claimTileButton);

        hotbarRecycler.setOnTouchListener(new View.OnTouchListener() {

            private final GestureDetector gestureDetector = new GestureDetector(context, new GestureDetector.SimpleOnGestureListener() {
                private boolean isOpen = false;
                @Override
                public boolean onDoubleTap(MotionEvent e) {
                    Log.d("TEST", "onDoubleTap");
                    if (isOpen) {
                        isOpen = false;
                        //android.widget.LinearLayout
                        hotbarRecycler.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 130,
                                context.getResources().getDisplayMetrics())));
                    }
                    else {
                        isOpen = true;
                        hotbarRecycler.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT));
                    }
                    return super.onDoubleTap(e);
                }
            });
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                gestureDetector.onTouchEvent(event);
                return false;
            }
        });
    }

    public void showHotbar() {
        runOnUiThread(() -> {
            hotbarRecycler.setVisibility(View.VISIBLE);
        });
    }

    public void hideHotbar() {
        //runOnUiThread(() -> {
        //    hotbarRecycler.setVisibility(View.GONE);
        //});
    }

    public void setHotbarItems(String jsonItemArray) {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            ArrayList<Item> list = new ArrayList<Item>();
            try {
                JSONArray jsonArray = (JSONArray) new JSONObject(jsonItemArray).getJSONArray("items");
                int len = jsonArray.length();
                for (int i = 0; i < len; i++) {
                    try {
                        JSONObject itemObj = (JSONObject) jsonArray.get(i);
                        String displayName = (String) itemObj.get("displayName");
                        int count = (int) itemObj.get("count");
                        String id = (String) itemObj.get("id");
                        list.add(new Item(count, displayName, id));
                    } catch (JSONException ignored) {

                    }
                }
            } catch (JSONException ignored) {

            }
            for (int i = 0; i < list.size(); i++) {
                Log.d("MainActivity", "Hotbar items " + String.valueOf(i) + " - " + list.get(i).getDisplayName());
            }
            hotbarItems.clear();
            hotbarItems.addAll(list);
            //hotbarAdpater.setDataset(hotbarItems);
            Log.d("MainActivity", "Hotbar items ds changed notify!");
            hotbarAdpater.notifyDataSetChanged();
        });
    }

    public void onHotbarItemClick(String itemId, int itemPosition) {
        Log.d("MainActivity", "onHotbarItem click : " + itemId);
        Item hotbarItem = hotbarItems.remove(itemPosition);
        hotbarItems.addFirst(hotbarItem);
        hotbarAdpater.notifyDataSetChanged();
        mView.eval("window.navigator.ui.callOnHotbarItemClick(\"" + itemId + "\")", "");
    }

    public void onMapCardClick(String id) {
        for (CardView value : mapCards.values()) {
            value.setBackgroundColor(0xFFFFFFFF);
        }
        CardView mapCard = mapCards.get(id);
        if (mapCard != null) {
            mapCard.setBackgroundColor(0xFF8888FF);
        }

        mView.eval("window.navigator.ui.callOnMapCardSelected(\"" + id + "\")", "");
    }

    public void addMapCard(String id, String text) {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            CardView mapCard = new CardView(this);
            TextView mapCardText = new TextView(this);
            mapCardText.setText(text);
            mapCard.addView(mapCardText);
            mapCard.setContentPadding(10, 10, 10, 10);
            mapCard.setBackgroundColor(0xFFFFFFFF);
            ConstraintLayout mapOverlay = (ConstraintLayout) findViewById(R.id.mapOverlay);
            mapOverlay.addView(mapCard);
            mapCard.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    onMapCardClick(id);
                }
            });
            mapCards.put(id, mapCard);
        });
    }

    public void removeMapCard(String id) {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            CardView mapCard = mapCards.get(id);
            if (mapCard != null) {
                ConstraintLayout mapOverlay = (ConstraintLayout) findViewById(R.id.mapOverlay);
                mapOverlay.removeView(mapCard);
                mapCards.remove(id, mapCard);
            }
        });
    }

    public void setMapCardTranslation(String id, float x, float y) {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            CardView mapCard = mapCards.get(id);
            if (mapCard != null) {
                mapCard.setTranslationX(0.5f*((x + 1f)*(float)mView.getWidth() - (float)mapCard.getWidth()));
                mapCard.setTranslationY(0.5f*((y + 1f)*(float)mView.getHeight() - (float)mapCard.getHeight()));
            }
        });
    }

    public void showLogin() {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            loginView.setVisibility(View.VISIBLE);
        });
        hideRegister();
    }

    public void hideLogin() {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            loginView.setVisibility(View.GONE);
        });
    }

    public void showRegister() {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
                    registerView.setVisibility(View.VISIBLE);
                });
        hideLogin();
    }

    public void hideRegister() {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            registerView.setVisibility(View.GONE);
        });
    }

    public void setLoginError(String errorText) {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            loginError.setText(errorText);
            loginError.setVisibility(View.VISIBLE);
        });
    }
    public void setRegisterError(String errorText) {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            registerError.setText(errorText);
            registerError.setVisibility(View.VISIBLE);
        });
    }

    public void clearLoginError() {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            loginError.setVisibility(View.INVISIBLE);
            loginError.setText("");
        });
    }

    public void clearRegisterError() {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            registerError.setVisibility(View.INVISIBLE);
            registerError.setText("");
        });
    }

    public void showTabView() {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            tabView.setVisibility(View.VISIBLE);
        });
        hideHomeContainer();
    }

    public void hideHomeContainer() {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            homeContainer.setVisibility(View.GONE);
        });
    }

    public void showClaimTile() {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            claimTileCard.setVisibility(View.VISIBLE);
            claimTileButton.setVisibility(View.VISIBLE);
            placeTileLayout.setVisibility(View.GONE);
            createTileLayout.setVisibility(View.GONE);
        });

    }

    public void hideClaimTile() {
        // Using runOnUiThread as this method is called via JNI
        runOnUiThread(() -> {
            claimTileCard.setVisibility(View.GONE);
            claimTileButton.setVisibility(View.GONE);
            placeTileLayout.setVisibility(View.GONE);
            createTileLayout.setVisibility(View.GONE);
        });

    }

    public void onClaimTile(View v) {
        claimTileCard.setVisibility(View.VISIBLE);
        claimTileButton.setVisibility(View.GONE);
        placeTileLayout.setVisibility(View.VISIBLE);
        createTileLayout.setVisibility(View.GONE);
        mView.eval("window.navigator.ui.callOnClaimCurrentTile(\"\")", "");
    }

    public void onCancelClaimTile(View v) {
        placeTileLayout.setVisibility(View.GONE);
        createTileLayout.setVisibility(View.GONE);
        claimTileButton.setVisibility(View.VISIBLE);
        mView.eval("window.navigator.ui.callOnCancelClaimCurrentTile(\"\")", "");
    }

    public void onCreateNewTile(View v) {
        placeTileLayout.setVisibility(View.GONE);
        createTileLayout.setVisibility(View.VISIBLE);
        claimTileButton.setVisibility(View.GONE);
    }

    public void onCancelCreateNewTile(View v) {
        createTileLayout.setVisibility(View.GONE);
        placeTileLayout.setVisibility(View.VISIBLE);
        claimTileButton.setVisibility(View.GONE);
    }

    public void onCreateNewTileConfirm(View v) {
        placeTileLayout.setVisibility(View.GONE);
        createTileLayout.setVisibility(View.GONE);
        claimTileButton.setVisibility(View.GONE);
        claimTileCard.setVisibility(View.GONE);
        mView.eval("window.navigator.ui.callOnCreateNewTile(\"\")", "");
    }

    //TODO: Add saved tile placing


    public void onTabSelectTiles(View v) {
        //TODO: hide tabs and show player tiles
        //TODO: send tab select event
        mView.eval("window.navigator.ui.callOnTabSelectTiles(\"\")", "");
    }

    public void onTabSelectMap(View v) {
        mView.eval("window.navigator.ui.callOnTabSelectMap(\"\")", "");
    }

    public void onTabSelectInventory(View v) {
        mView.eval("window.navigator.ui.callOnTabSelectInventory(\"\")", "");
    }

    public void onTabSelectCamera(View v) {
        mView.eval("window.navigator.ui.callOnTabSelectCamera(\"\")", "");
    }

    public void setTextOverlayText(String text) {
        runOnUiThread(() -> {
            textOverlay.setText(text);
            textOverlay.setVisibility(View.VISIBLE);
        });
    }

    public void setTextOverlayTransform(float x, float y, float scale) {
        runOnUiThread(() -> {
            textOverlay.setTranslationX(0.5f*((x + 1f)*(float)mView.getWidth() - (float)textOverlay.getWidth()));
            textOverlay.setTranslationY(0.5f*((y + 1f)*(float)mView.getHeight() - (float)textOverlay.getHeight()));
            textOverlay.setScaleX(scale);
            textOverlay.setScaleY(scale);
        });
    }

    public void hideTextOverlay() {
        runOnUiThread(() -> {
            textOverlay.setVisibility(View.GONE);
        });
    }

    public void showTextOverlay() {
        runOnUiThread(() -> {
            textOverlay.setVisibility(View.VISIBLE);
        });
    }


    public void onStartLogin(View v) {
        showLogin();
    }

    public void onStartRegister(View v) {
        showRegister();
    }

    public void onCancelRegister(View v) {
        hideRegister();
    }

    public void onLogin(View v) {
        LinearLayout loginLayout = (LinearLayout) loginView.findViewById(R.id.loginLayout);
        EditText usernameText = (EditText) loginLayout.findViewById(R.id.editTextUsername);
        EditText passwordText = (EditText) loginLayout.findViewById(R.id.editTextPassword);
        mView.eval("BABYLON.Tools.Log(\"onLoginBJS\")", "");
        try {
            mView.eval("BABYLON.Tools.Log(\"onLoginFJS\")", "");
            String source = new JSONObject().put("username", usernameText.getText().toString())
                    .put("password", passwordText.getText().toString())
                    .toString();
            mView.eval("window.navigator.ui.callOnLogin('" + source + "')", "");
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void onRegister(View v) {
        LinearLayout registerLayout = (LinearLayout) registerView.findViewById(R.id.registerLayout);
        EditText usernameText = (EditText) registerLayout.findViewById(R.id.editTextRegisterUsername);
        EditText emailText = (EditText) registerLayout.findViewById(R.id.editTextRegisterEmail);
        EditText passwordText1 = (EditText) registerLayout.findViewById(R.id.editTextRegisterPassword1);
        EditText passwordText2 = (EditText) registerLayout.findViewById(R.id.editTextRegisterPassword2);
        mView.eval("BABYLON.Tools.Log(\"onRegisterBJS\")", "");
        try {
            mView.eval("BABYLON.Tools.Log(\"onRegisterFJS\")", "");
            String source = new JSONObject().put("username", usernameText.getText().toString())
                    .put("password1", passwordText1.getText().toString())
                    .put("password2", passwordText2.getText().toString())
                    .put("email", emailText.getText().toString())
                    .toString();
            mView.eval("window.navigator.ui.callOnRegister('" + source + "')", "");
        } catch (JSONException e) {
            e.printStackTrace();
        }

    }

    public void onCancelLogin(View v) {
        hideLogin();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus && mView.getVisibility() == View.GONE) {
            mView.setVisibility(View.VISIBLE);
        }
    }

    @Override
    protected void onPause() {
        mView.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    @Override
    public void onViewReady() {
        mView.setCurrentActivity(this);
        mView.loadScript("app:///Scripts/index.js");
        mView.eval("BABYLON.Tools.Log(\"ActUI test\");", "");
        mView.eval("window.navigator.ui.callOnUIAvailable(\"\")", "");
    }
}