Feature: System States

    Scenario: POR into Off
        Given off setup

        When I enable schedule 'RearLightSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Off' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I wait '100' milliseconds
        And I enable 'PSU' output

        Then the brightness should be '0'

    Scenario: POR into Adaptive
        Given off setup

        When I enable schedule 'RearLightSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Adaptive' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '60' on 'LineMaster'
        And I wait '100' milliseconds
        And I enable 'PSU' output

        Then the brightness should be '60'

    Scenario: POR into Standard
        Given off setup

        When I enable schedule 'RearLightSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Standard' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '60' on 'LineMaster'
        And I wait '100' milliseconds
        And I enable 'PSU' output

        Then the brightness should be '60'

    Scenario: POR into Emergency
        Given off setup

        When I enable schedule 'RearLightSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Emergency' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I wait '100' milliseconds
        And I enable 'PSU' output

        Then the brightness should be '30'

    Scenario: POR into Safety
        Given on setup

        When I enable 'PSU' output
        And I wait '5' seconds

        #Then the brightness should be 'SafetyBrightness'

    Scenario: Normal into Safety
        Given on setup

        When I enable schedule 'RearLightSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Standard' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '10' on 'LineMaster'

        Then the brightness should be '20'

        When I disable schedule on 'LineMaster'
        And I wait '5' seconds

        #Then the brightness should be 'SafetyBrightness'

    Scenario: Safety into Normal
        Given on setup

        When I wait '5' seconds

        #Then the brightness should be 'SafetyBrightness'

        When I enable schedule 'RearLightSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Standard' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '10' on 'LineMaster'

        #Then the brightness should be '10'

    # TODO: emergency into safety
    Scenario: Emergency into Safety
        Given on setup

        When I enable schedule 'RearLightSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Emergency' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '0' on 'LineMaster'
        And I wait '500' milliseconds

        #Then the brightness should be 'Emergency'

        When I disable schedule on 'LineMaster'
        And I wait '5' seconds

        #Then the brightness should be 'Emergency'