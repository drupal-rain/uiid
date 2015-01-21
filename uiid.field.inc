<?php

/**
 *  Implements hook_field_info().
 */
function uiid_field_info() {
  return array(
    'uiid' => array(
      'label' => t('UIID'),
      'description' => t("This field stores the UIID value"),
      'settings' => array(),
      'instance_settings' => array(
        'source' => '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ',
        'min' => 5,
        'max' => 5,
        'initiate' => TRUE,
        'manual' => FALSE,
        'revisable' => FALSE,
      ),
      'default_widget' => 'uiid_default',
      'default_formatter' => 'uiid_default',
    ),
  );
}

/**
 *  Implements hook_field_instance_settings_form().
 */
function uiid_field_instance_settings_form($field, $instance) {
  $settings = $instance['settings'];

  $form = array();
  $form['source'] = array(
    '#type' => 'textarea',
    '#title' => t('Source characters'),
    '#rows' => 3,
    '#default_value' => $settings['source'],
  );
  $form['min'] = array(
    '#type' => 'textfield',
    '#title' => t('Minimum length'),
    '#size' => 3,
    '#default_value' => $settings['min'],
  );
  $form['max'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum length'),
    '#size' => 3,
    '#default_value' => $settings['max'],
  );
  $form['initiate'] = array(
    '#type' => 'checkbox',
    '#title' => t('Initiate on start'),
    '#default_value' => $settings['initiate']
  );
  if ($field['cardinality'] == 1) {
    $form['revisable'] = array(
      '#type' => 'checkbox',
      '#title' => t('Revisable'),
      '#default_value' => $settings['revisable']
    );
  }

  return $form;
}

/**
 *  Implements hook_field_is_empty().
 */
function uiid_field_is_empty($item, $field) {
  if (empty($item['value'])) {
    return TRUE;
  }

  return FALSE;
}

/**
 *  Implements hook_field_widget_info().
 */
function uiid_field_widget_info() {
  return array(
    'uiid_default' => array(
      'label' => t('UIID input'),
      'field types' => array('uiid'),
    ),
  );
}

/**
 *  Implements hook_field_widget_form().
 */
function uiid_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  $settings = $instance['settings'];
  // Item value has two formats:
  // - $items[0]['value'] from db to entity.
  // - $items[0]['value]-['value']/['generate'] from input for field edit UI.
  $value = NULL;
  if (isset($items[$delta]['value'])) {
    if (is_array($items[$delta]['value'])) {
      $value = $items[$delta]['value']['value'];
    }
    else {
      $value = $items[$delta]['value'];
    }
  }

  $element += array(
    '#type' => 'uiid',
    '#default_value' => $value,
    '#uiid' => array(
      'source' => $settings['source'],
      'max' => $settings['max'],
      'min' => $settings['min'],
      'revisable' => $settings['revisable'],
      'unique_callback' => 'uiid_field_unique_callback',
    ),
  );
  // Deal with instance settings for entity form.
  if (isset($element['#entity'])) {
    if (!isset($value) && $settings['initiate']) {
      $element['#default_value'] = uiid_generate($element, $form_state);
    }
    if (
      $field['cardinality'] == 1
      && isset($value) && !$settings['revisable']
    ) {
      $element['#uiid']['generate'] = FALSE;
    }
  }

  return $element;
}

/**
 *  Implements hook_field_presave().
 */
function uiid_field_presave($entity_type, $entity, $field, $instance, $langcode, &$items) {
  // @todo
  // Seems I don't need to, the Drupal Form API handle it for me.
  /*
  if ($field['cardinality'] == 1 && !$instance['settings']['revisable']) {
    $items_original = field_get_items($entity_type, $entity->original, $field['field_name'], $langcode);
    if (
      (count($items) == 0 && count($items_original) == 1)
      || (count($items_original) == 1 && $items[0]['value'] != $items_original[0]['value'])
    ) {
      dsm($items_original);
      dsm("I Know what you have done.");
    }
  }
  */
}

/**
 *  Implements hook_field_formatter_info().
 */
function uiid_field_formatter_info() {
  return array(
    'uiid_default' => array(
      'label' => t('UIID text'),
      'field types' => array('uiid'),
    ),
  );
}

/**
 *  Implements hook_field_formatter_view().
 */
function uiid_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  foreach ($items as $delta => $item) {
    $element[$delta] = array('#markup' => $item['value']);
  }

  return $element;
}

/**
 * Callback function for uiid element.
 */
function uiid_unique_callback($value, $element, $form_state) {
  $new_value = FALSE;
  if ($value != $element['#default_value']) {
    $new_value = TRUE;
  }
  $field_state = field_form_get_state($element['#field_parents'], $element['#field_name'], $element['#language'], $form_state);
  $query = new EntityFieldQuery();
  $query->fieldCondition($field_state['field'], 'value', $value);
  $query->count();
  $exists = $query->execute();

  if ($new_value && $exists) {
    return FALSE;
  }
  return TRUE;
}
