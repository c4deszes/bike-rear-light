Feature: Operation status

    Scenario: Normal operation
        Given on setup

        When I enable schedule 'RearLightSchedule' on 'LineMaster'
        And I set master signal 'LightSynchronization.LightMode' to 'Standard' on 'LineMaster'
        And I set master signal 'LightSynchronization.TargetBrightness' to '50' on 'LineMaster'

        And I wait '2' seconds
        And I request operation status of 'RearLight' on 'LineMaster'
        And I wait '2' seconds

        Then the operation status of 'RearLight' should be 'Ok' on 'LineMaster'

    # TODO: test short/open circuit op status
