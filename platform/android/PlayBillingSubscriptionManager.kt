package com.ficamotor.platform

import android.app.Activity
import android.content.Context
import com.android.billingclient.api.*

/** Product ID: must match Google Play Console. */
private const val PRODUCT_ID = "tracking_premium"

/** Preferred trial periods (ISO 8601): P1W = 7 days, P7D = 7 days. Main marketing hook. */
private val TRIAL_PERIOD_7_DAYS = setOf("P1W", "P7D")

/**
 * Google Play Billing subscription manager.
 * Product: tracking_premium — auto-renewable, 7-day free trial, $4.99 / R$ 29.90.
 * Explicitly selects the 7-day trial offer (offerToken) for marketing parity with iOS.
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
                        subscriptionActive = purchases.any { it.products.contains(PRODUCT_ID) }
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
                        .setProductId(PRODUCT_ID)
                        .setProductType(BillingClient.ProductType.SUBS)
                        .build()
                )
            )
            .build()
        billingClient.queryProductDetailsAsync(params) { result, productDetailsList ->
            val product = productDetailsList?.firstOrNull() ?: return@queryProductDetailsAsync
            val offers = product.subscriptionOfferDetails ?: return@queryProductDetailsAsync
            val trialOffer = selectTrialOffer(offers) ?: offers.firstOrNull()
            trialOffer?.let { offer ->
                val flowParams = BillingFlowParams.newBuilder()
                    .setProductDetailsParamsList(
                        listOf(
                            BillingFlowParams.ProductDetailsParams.newBuilder()
                                .setProductDetails(product)
                                .setOfferToken(offer.offerToken)
                                .build()
                        )
                    )
                    .build()
                billingClient.launchBillingFlow(activity, flowParams)
            }
        }
    }

    /**
     * Select the 7-day trial offer. Marketing parity with iOS.
     * Prefer offer with free trial phase and P1W/P7D (7 days).
     * Fallback: any offer with a free phase.
     */
    private fun selectTrialOffer(offers: List<ProductDetails.SubscriptionOfferDetails>): ProductDetails.SubscriptionOfferDetails? {
        // Prefer 7-day trial (P1W or P7D)
        offers.firstOrNull { hasFreeTrialWith7DayPeriod(it) }?.let { return it }
        // Fallback: any offer with free trial phase
        return offers.firstOrNull { hasFreeTrial(it) }
    }

    private fun hasFreeTrial(offer: ProductDetails.SubscriptionOfferDetails): Boolean {
        return offer.pricingPhases.pricingPhaseList.any { it.priceAmountMicros == 0L }
    }

    private fun hasFreeTrialWith7DayPeriod(offer: ProductDetails.SubscriptionOfferDetails): Boolean {
        return offer.pricingPhases.pricingPhaseList.any { phase ->
            phase.priceAmountMicros == 0L && phase.billingPeriod in TRIAL_PERIOD_7_DAYS
        }
    }
}
