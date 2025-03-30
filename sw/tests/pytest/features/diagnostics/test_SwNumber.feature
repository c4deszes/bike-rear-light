Feature: Software Number

    Scenario: Get Software number
        Given on setup

        And I wait '1' seconds
        And I request software version of 'RearLight' on 'LineMaster'
        And I wait '1' seconds

        Then the software version of 'RearLight' should be '1.0.0' on 'LineMaster'
