# Mother Bird

Mother Bird is designed to run on a [Particle](https://particle.io) [Argon](https://docs.particle.io/datasheets/wi-fi/argon-datasheet/) board.  It listens to baby birds on its mesh network and manages them.

## Message Subscriptions

### `moisture-check`

The `moisture-check` message is sent from baby birds and contains the following payload:

```JS
{
    "deviceName": string,
    "moistureLevel": number
}
```
It then calculates whether the baby bird should activate or deactivate watering its planter box and responds via the `hydration-needed` message.


## Message Announcements

### `hydration-needed`

The `hydration-needed` message is sent on receipt of any `moisture-check` message.  It is sent to notify a specific baby bird whether it should consider itself hydrated or dehydrated.

```JS
{
    "deviceName": string,
    "shouldHydrate": bool
}
```

