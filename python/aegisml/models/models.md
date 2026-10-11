# Training pipelines

```yaml
Supervised Dataset
       |
       v
Chronological Split
       |
       +-----------> Training Data
       |                  |
       |                  v
       |            StandardScaler
       |                  |
       |                  v
       |          Logistic Regression
       |
       +-----------> Validation Data
       |                  |
       |                  v
       |           Model Evaluation
       |
       +-----------> Test Data
                          |
                          v
                   Final Evaluation
```

we are establishing baseline model, we will focus on other aspects as we discover more models.