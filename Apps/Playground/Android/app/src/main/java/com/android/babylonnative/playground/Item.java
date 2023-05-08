package com.android.babylonnative.playground;

public class Item {
    private int count;
    private String displayName;
    private String id;

    public Item (int count, String displayName, String id) {
        this.count = count;
        this.displayName = displayName;
        this.id = id;
    }

    public String getDisplayName() {
        return displayName;
    }

    public String getId() {
        return id;
    }

    public int getCount() {
        return count;
    }
}
