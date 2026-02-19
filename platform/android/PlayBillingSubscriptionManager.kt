package com.ficamotor.platform

import android.app.Activity
import android.content.Context
import com.android.billingclient.api.*

/**
 * Google Play Billing subscription manager.
 * Product: auto-renewable, 7-day trial, $4.99 / R$ 29.90.
 * Receipt validation: use Purchase.getOriginalJson() + server-side validation.
 */
class PlayBillingSubscriptionManager(
    private val context: Context,
    private val activity: Activity
) : PurchasesUpdatedListener {

    private var billingClient: BillingClient = BillingClient.newBuilder(context)
        .setListener(this)
        .enablePendingPurchases()
        .build()

    private var onStatusChanged: ((Boolean) -> Unit)? = null
    private var subscriptionActive = false

    fun setOnStatusChanged(callback: (Boolean) -> Unit) {
        onStatusChanged = callback
    }

    fun isSubscriptionActive(): Boolean = subscriptionActive

    fun purchase() {
        billingClient.startConnection(object : BillingClientStateListener {
            override fun onBillingSetupFinished(result: BillingResult) {
                if (result.responseCode == BillingClient.BillingResponseCode.OK) {
                    queryProductAndLaunch(activity)
                }
            }
            override fun onBillingServiceDisconnected() {}
        })
    }

    fun restore() {
        billingClient.startConnection(object : BillingClientStateListener {
            override fun onBillingSetupFinished(result: BillingResult) {
                if (result.responseCode == BillingClient.BillingResponseCode.OK) {
                    billingClient.queryPurchasesAsync(BillingClient.ProductType.SUBS) { _, purchases ->
                        subscriptionActive = purchases.any { it.products.contains("tracking_premium") }
                        onStatusChanged?.invoke(subscriptionActive)
                    }
                }
            }
            override fun onBillingServiceDisconnected() {}
        })
    }

    override fun onPurchasesUpdated(result: BillingResult, purchases: List<Purchase>?) {
        if (result.responseCode == BillingClient.BillingResponseCode.OK && purchases != null) {
            for (p in purchases) {
                if (p.purchaseState == Purchase.PurchaseState.PURCHASED) {
                    subscriptionActive = true
                    onStatusChanged?.invoke(true)
                }
            }
        }
    }

    private fun queryProductAndLaunch(activity: Activity) {
        val params = QueryProductDetailsParams.newBuilder()
            .setProductList(
                listOf(
                    QueryProductDetailsParams.Product.newBuilder()
                        .setProductId("tracking_premium")
                        .setProductType(BillingClient.ProductType.SUBS)
                        .build()
                )
            )
            .build()
        billingClient.queryProductDetailsAsync(params) { result, productDetailsList ->
            productDetailsList.firstOrNull()?.subscriptionOfferDetails?.firstOrNull()?.let { offer ->
                val flowParams = BillingFlowParams.newBuilder()
                    .setProductDetailsParamsList(
                        listOf(
                            BillingFlowParams.ProductDetailsParams.newBuilder()
                                .setProductDetails(productDetailsList.first())
                                .setOfferToken(offer.offerToken)
                                .build()
                        )
                    )
                    .build()
                billingClient.launchBillingFlow(activity, flowParams)
            }
        }
    }
}
