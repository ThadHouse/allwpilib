// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

// THIS FILE WAS AUTO-GENERATED BY ./ntcore/generate_topics.py. DO NOT MODIFY

namespace NetworkTables;

/** NetworkTables generic publisher. */
public interface GenericPublisher : Publisher {

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool Set(in NetworkTableValue value);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   * @throws IllegalArgumentException if the value is not a known type
   */
  bool SetValue<T>(T value) {
    return SetValue(value, 0);
  }

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   * @throws IllegalArgumentException if the value is not a known type
   */
  bool SetValue<T>(T value, long time);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool SetBoolean(bool value) {
    return SetBoolean(value, 0);
  }
  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   */
  bool SetBoolean(bool value, long time);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool SetInteger(long value) {
    return SetInteger(value, 0);
  }
  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   */
  bool SetInteger(long value, long time);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool SetFloat(float value) {
    return SetFloat(value, 0);
  }
  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   */
  bool SetFloat(float value, long time);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool SetDouble(double value) {
    return SetDouble(value, 0);
  }
  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   */
  bool SetDouble(double value, long time);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool SetString(string value) {
    return SetString(value, 0);
  }
  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   */
  bool SetString(string value, long time);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool SetRaw(byte[] value) {
    return SetRaw(value, 0);
  }
  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   */
  bool SetRaw(byte[] value, long time);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool SetBooleanArray(bool[] value) {
    return SetBooleanArray(value, 0);
  }
  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   */
  bool SetBooleanArray(bool[] value, long time);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool SetIntegerArray(long[] value) {
    return SetIntegerArray(value, 0);
  }
  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   */
  bool SetIntegerArray(long[] value, long time);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool SetFloatArray(float[] value) {
    return SetFloatArray(value, 0);
  }
  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   */
  bool SetFloatArray(float[] value, long time);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool SetDoubleArray(double[] value) {
    return SetDoubleArray(value, 0);
  }
  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   */
  bool SetDoubleArray(double[] value, long time);

  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @return False if the topic already exists with a different type
   */
  bool SetStringArray(string[] value) {
    return SetStringArray(value, 0);
  }
  /**
   * Publish a new value.
   *
   * @param value value to publish
   * @param time timestamp; 0 indicates current NT time should be used
   * @return False if the topic already exists with a different type
   */
  bool SetStringArray(string[] value, long time);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefault(in NetworkTableValue defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   * @throws IllegalArgumentException if the value is not a known type
   */
  bool SetDefaultValue<T>(T defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefaultBoolean(bool defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefaultInteger(long defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefaultFloat(float defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefaultDouble(double defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefaultString(string defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefaultRaw(byte[] defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefaultBooleanArray(bool[] defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefaultIntegerArray(long[] defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefaultFloatArray(float[] defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefaultDoubleArray(double[] defaultValue);

  /**
   * Sets the entry's value if it does not exist.
   *
   * @param defaultValue the default value to set
   * @return False if the entry exists with a different type
   */
  bool SetDefaultStringArray(string[] defaultValue);

}