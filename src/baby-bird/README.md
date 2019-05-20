# Baby Bird

Baby Bird is designed to run on a [Particle](https://particle.io) [Xenon](https://docs.particle.io/datasheets/mesh/xenon-datasheet/) board.  It monitors soil moisture and reports to its mother bird every 2 seconds.  It also activates an attached solenoid valve if the mother bird notifies it that it is dehydrated.

## Message Subscriptions

### `hydration-needed`

The `hydration-needed` message is sent from mother birds and contains the following payload:

```JS
{
    "deviceName": string,
    "shouldHydrate": bool
}
```

If `shouldHydrate` is true, the baby bird will activate an LED and solenoid valve to allow water into the planter box.  If false, the baby bird will deactivate the LED and close the solenoid valve.


## Message Announcements

### `moisture-check`

The `moisture-check` message is sent by the baby bird to its mother bird every 2 seconds and contains the following payload:

```JS
{
    "deviceName": string,
    "moistureLevel": number
}
```
