<?php

/**
 * @file
 * Stanford Alt Check wysiwyg form.
 */

/**
 * Menu callback to display list of entities with missing alt attributes.
 */
function stanford_alt_check_image_wysiwyg($form, &$form_state) {
  drupal_set_title(t("WYSIWYG Images"));
  $form = array();
  $form['#tree'] = TRUE;

  $image_entities = stanford_alt_check_get_wysiwyg();
  foreach ($image_entities as $field => $types) {
    foreach ($types as $type => $bundles) {
      foreach ($bundles as $entities) {
        foreach ($entities as $id => $field_items) {
          foreach ($field_items as $delta => $images) {
            $entity = entity_load($type, array($id));
            $entity = reset($entity);
            foreach ($images as $position => $image) {
              $image_row = stanford_alt_check_get_image_form_component($image, $entity, $type);
              if ($image_row) {
                $form['images'][$field][$type][$id][$delta][$position] = $image_row;
              }
            }
          }
        }
      }
    }
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Save the image new alt text.
 */
function stanford_alt_check_image_wysiwyg_submit($form, &$form_state) {
  $changed = 0;
  foreach ($form_state['values']['images'] as $field_name => $types) {
    foreach ($types as $type => $entities) {
      foreach ($entities as $id => $field_items) {
        $entity = NULL;
        foreach ($field_items as $delta => $images) {
          foreach ($images as $position => $alt) {

            // No alt provided.
            if (!$alt['alt']) {
              continue;
            }

            // Current alt matches existing alt.
            if ($form['images'][$field_name][$type][$id][$delta][$position]['alt']['#default_value'] == $alt['alt']) {
              continue;
            }

            $changed++;
            $entity = entity_load($type, array($id));
            $entity = reset($entity);

            stanford_alt_check_set_wysiwyg_alt($entity->{$field_name}[LANGUAGE_NONE][$delta], $position, $alt['alt']);
          }

          if ($entity) {
            entity_save($type, $entity);
          }
        }
      }
    }
  }

  drupal_set_message(t('Alt Attributes updated for !num images', array('!num' => $changed)));
}

/**
 * Use DOMDocument to find the image and set the alt text.
 *
 * @param array $field
 *   Field item data.
 * @param int $position
 *   The position of the image in the html as found by
 *   DOMDocument->getElementsByTagName('img').
 * @param string $alt
 *   The alt attribut string to set.
 */
function stanford_alt_check_set_wysiwyg_alt(array &$field, $position, $alt) {
  $dom = new DOMDocument();

  $dom->loadHTML($field['value'], LIBXML_HTML_NOIMPLIED | LIBXML_HTML_NODEFDTD);
  /** @var DOMElement $img_element */
  foreach ($dom->getElementsByTagName('img') as $pos => $img_element) {
    if ($position != $pos) {
      continue;
    }
    $img_element->setAttribute('alt', $alt);
  }
  $field['value'] = $dom->saveHTML();

  if (isset($field['safe_value'])) {
    $dom->loadHTML($field['safe_value'], LIBXML_HTML_NOIMPLIED | LIBXML_HTML_NODEFDTD);
    /** @var DOMElement $img_element */
    foreach ($dom->getElementsByTagName('img') as $pos => $img_element) {
      if ($position != $pos) {
        continue;
      }
      $img_element->setAttribute('alt', $alt);
    }
    $field['safe_value'] = $dom->saveHTML();
  }
}
