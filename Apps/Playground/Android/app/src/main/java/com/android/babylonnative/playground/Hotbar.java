package com.android.babylonnative.playground;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.List;

public class Hotbar extends RecyclerView.Adapter<Hotbar.ViewHolder> {

    private final List<Item> localDataSet;

    private final MainActivity mainActivity;

    /**
     * Provide a reference to the type of views that you are using
     * (custom ViewHolder)
     */
    public static class ViewHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        private final TextView textView;

        private final MainActivity mainActivity;

        private Item data;

        private int position;

        public ViewHolder(View view, MainActivity mainActivity) {
            super(view);

            this.mainActivity = mainActivity;
            // Define click listener for the ViewHolder's View
            view.setOnClickListener(this);
            textView = (TextView) view.findViewById(R.id.itemTextView);
        }

        public TextView getTextView() {
            return textView;
        }

        public void setData(Item item, int itemIndex) {
            this.position = itemIndex;
            this.data = item;
            this.textView.setText(item.getDisplayName());
        }

        @Override
        public void onClick(View v) {
            Log.d("Hotbar Recycler VH", "onClick");
            Log.d("Hotbar Recycler VH", "onClick : " + data.getId());
            mainActivity.onHotbarItemClick(data.getId(), position);
        }
    }

    /**
     * Initialize the dataset of the Adapter
     *
     * @param dataSet String[] containing the data to populate views to be used
     * by RecyclerView
     */
    public Hotbar(List<Item> dataSet, MainActivity mainActivity) {
        this.mainActivity = mainActivity;
        localDataSet = dataSet;

    }

    public void setDataset(ArrayList<Item> dataSet) {
        Log.d("Hotbar Recycler", "Hotbar setDataset!");
        localDataSet.clear();
        localDataSet.addAll(dataSet);
    }

    // Create new views (invoked by the layout manager)
    @Override
    public ViewHolder onCreateViewHolder(ViewGroup viewGroup, int viewType) {
        Log.d("Hotbar Recycler", "Hotbar onCreateViewHolder");
        // Create a new view, which defines the UI of the list item
        View view = LayoutInflater.from(viewGroup.getContext())
                .inflate(R.layout.item, viewGroup, false);

        return new ViewHolder(view, mainActivity);
    }

    // Replace the contents of a view (invoked by the layout manager)
    @Override
    public void onBindViewHolder(ViewHolder viewHolder, final int position) {
        Log.d("Hotbar Recycler", "Hotbar onBindViewHolder!");

        // Get element from your dataset at this position and replace the
        // contents of the view with that element
        //viewHolder.getTextView().setText(localDataSet.get(position).getDisplayName());
        viewHolder.setData(localDataSet.get(position), position);
    }

    // Return the size of your dataset (invoked by the layout manager)
    @Override
    public int getItemCount() {
        return localDataSet.size();
    }
}