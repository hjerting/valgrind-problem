#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VALUE_ACE 14
#define VALUE_KING 13
#define VALUE_QUEEN 12
#define VALUE_JACK 11
#define DECK_SIZE 52
#define CHAR_LIMIT 4
#define LAST CHAR_LIMIT - 1

typedef enum {
  SPADES,
  HEARTS,
  DIAMONDS,
  CLUBS,
  NUM_SUITS
} suit_t;

typedef struct card_tag
{
    unsigned value;
    suit_t suit;
} card_t;

typedef struct deck_tag
{
	card_t ** cards;
	size_t n_cards;
} deck_t;

typedef struct future_cards_tag
{
    deck_t * decks;
    size_t n_decks;
} future_cards_t;

void free_decks(deck_t **decks, size_t n_decks);
deck_t ** read_input(FILE * f, size_t * n_hands, future_cards_t * fc);
deck_t *generate_new_deck();
void future_cards_from_deck(deck_t * deck, future_cards_t * fc);
void free_deck(deck_t * deck);
void print_hand(deck_t * hand);
void free_future_cards(future_cards_t *fc);

int main(void)
{
    char *filename = "test1.txt";
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        fprintf(stderr, "Failed to open file '%s'. Error: %d\n", filename, errno);
        return EXIT_FAILURE;
    }

    future_cards_t *fc = malloc(sizeof(*fc));
    if (fc == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for future cards. Error: %d\n", errno);
        fclose(f);
        return EXIT_FAILURE;
    }
    fc->decks = NULL;
    fc->n_decks = 0;

    size_t n_hands = 0;
    deck_t **hands = read_input(f, &n_hands, fc);

    if (hands == NULL)
    {
        printf("Hands == NULL\n");
    }
    else {
        deck_t *new_deck = generate_new_deck();
        future_cards_from_deck(new_deck, fc);
        free_deck(new_deck);
        for (int i = 0; i < n_hands; ++i)
        {
            print_hand(hands[i]);
            printf("\n");
        }
    }
    
    fclose(f);
    free_future_cards(fc);
    free_decks(hands, n_hands);
}

void free_decks(deck_t **decks, size_t n_decks)
{
  if (decks == NULL) return;
  for (size_t i = 0; i < n_decks; ++i)
  {
    free_deck(decks[i]);
  }
  free(decks);
}

char value_letter(card_t c)
{
	switch (c.value)
	{
		case VALUE_ACE:
			return 'A';
		case VALUE_KING:
			return 'K';
		case VALUE_QUEEN:
			return 'Q';
		case VALUE_JACK:
			return 'J';
		case 10:
			return '0';
		default:
			if (c.value >= 2 && c.value <= 9)
				return c.value + '0';
	}
	return '~';
}

char suit_letter(card_t c)
{
	switch (c.suit)
	{
		case SPADES:
			return 's';
		case HEARTS:
			return 'h';
		case DIAMONDS:
			return 'd';
		case CLUBS:
			return 'c';
		default:
			return '~';
	}
	return '~';
}

void print_card(card_t c)
{
	printf("%c%c", value_letter(c), suit_letter(c));
}

int is_white_space(char c)
{
    return((c >= 9 && c <= 13) || c == 32 || c == 133 || c == 160);
}

char *trim_hand(const char *str)
{
    size_t start = 0;
    while (is_white_space(str[start]))
    {
        ++start;
    }
    if (str[start] == '\0')
    {
        return NULL;
    }
    size_t end = strlen(str) - 1;
    while (is_white_space(str[end]))
    {
        --end;
    }
    char *trimmed = malloc(sizeof(*trimmed) * (end - start + 2));
    size_t i = 0;
    while (start <= end)
    {
        trimmed[i++] = str[start++];
    }
    trimmed[i] = '\0';
    return trimmed;
}

deck_t *initialize_deck()
{
  deck_t *deck = malloc(sizeof(*deck));
  if (deck == NULL)
  {
    fprintf(stderr, "Failed to allocate memory for new deck. Error: %d\n", errno);
    return NULL;
  }
  deck->n_cards = 0;
  deck->cards = NULL;
  return deck;
}

void add_card_to(deck_t * deck, card_t c)
{
/* Add the particular card to the given deck (which will
   involve reallocing the array of cards in that deck).
 */
  card_t *new_card = malloc(sizeof(*new_card));
  if (new_card == NULL)
  {
    fprintf(stderr, "Could not allocate memory for a new card. Error: %d\n", errno);
    return;
  }
  card_t **new_cards = realloc(deck->cards, sizeof(*new_cards) * (deck->n_cards + 1));
  if (new_cards == NULL)
  {
    fprintf(stderr, "Could not allocate memory for larger deck. Error: %d\n", errno);
    free(new_card);
    return;
  }
  new_card->suit = c.suit;
  new_card->value = c.value;
  deck->cards = new_cards;
  deck->cards[deck->n_cards] = new_card;
  ++deck->n_cards;
}

card_t * add_empty_card(deck_t * deck)
{
/* Add a card whose value and suit are both 0, and return a pointer
   to it in the deck.
   This will add an invalid card to use as a placeholder
   for an unknown card.
  */
  card_t *new_card = malloc(sizeof(*new_card));
  if (new_card == NULL)
  {
    fprintf(stderr, "Could not allocate memory for a new card. Error: %d\n", errno);
    return NULL;
  }
  card_t **new_cards = realloc(deck->cards, sizeof(*new_cards) * (deck->n_cards + 1));
  if (new_cards == NULL)
  {
    fprintf(stderr, "Could not allocate memory for larger deck. Error: %d\n", errno);
    free(new_card);
    return NULL;
  }
  new_card->suit = 0;
  new_card->value = 0;
  deck->cards = new_cards;
  deck->cards[deck->n_cards] = new_card;
  ++deck->n_cards;
  return new_card;
}

void add_card_pointer_to_deck(deck_t *deck, card_t *p)
{
  card_t **cards = realloc(deck->cards, sizeof(*cards) * (deck->n_cards + 1));
  if (cards == NULL)
  {
    fprintf(stderr, "Failed to allocate memory for larger deck of card pointers. Error: %d\n", errno);
    return;
  }
  deck->cards = cards;
  deck->cards[deck->n_cards] = p;
  ++deck->n_cards;
}

void add_future_card(future_cards_t * fc, size_t index, card_t * ptr)
{
    deck_t *new_decks = NULL;
    deck_t *new_deck = NULL;
    while (index >= fc->n_decks)
    {
        new_deck = initialize_deck();
        if (new_deck == NULL)
        {
            fprintf(stderr, "Memory failed to add deck to future cards. Error %d\n", errno);
            return;
        }
        new_decks = realloc(fc->decks, sizeof(*fc->decks) * (fc->n_decks + 1));
        if (new_decks == NULL)
        {
            fprintf(stderr, "Failed to allocate more memory to future card deck. Error: %d\n", errno);
            return;
        }
        fc->decks = new_decks;
        fc->decks[fc->n_decks] = *new_deck;
        ++fc->n_decks;
    }
    add_card_pointer_to_deck(&fc->decks[index], ptr);
}

int is_card_valid(card_t card)
{
	return (card.value >= 2 && card.value <= VALUE_ACE) && (card.suit >= 0 && card.suit < NUM_SUITS);
}

void assert_card_valid(card_t c)
{
	assert(is_card_valid(c));
}

int value_to_int(char letter)
{
	switch (letter)
	{
		case 'A':
			return VALUE_ACE;
		case 'K':
			return VALUE_KING;
		case 'Q':
			return VALUE_QUEEN;
		case 'J':
			return VALUE_JACK;
		case '0':
			return 10;
		default:
			if (letter >= '2' && letter <= '9')
				return letter - '0';
	}
	return '\0';
}

int suit_to_int(char letter)
{
	switch (letter)
	{
		case 's':
			return SPADES;
		case 'h':
			return HEARTS;
		case 'd':
			return DIAMONDS;
		case 'c':
			return CLUBS;
	}
	return 1000;
}

card_t card_from_letters(char value_let, char suit_let)
{
  card_t temp;
  temp.value = value_to_int(value_let);
  temp.suit = suit_to_int(suit_let);
  assert_card_valid(temp);
  return temp;
}

deck_t * hand_from_string(const char * str, future_cards_t * fc)
{
    deck_t *hand = initialize_deck();
    card_t card;
    card_t *card_p;
    char c[CHAR_LIMIT];

    size_t start = 0;
    size_t end;
    size_t length = strlen(str);
    size_t i;
    int index;

    while (start < length)
    {
        if (is_white_space(str[start]))
        {
            ++start;
        }
        end = start;
        while (!is_white_space(str[end]) && str[end] != '\0')
        {
            ++end;
        }
        if (start < end)
        {
            i = 0;
            while (start < end && i < LAST)
            {
                c[i++] = str[start++];
            }
            c[i] = '\0';
        }
        if (c[0] == '?')
        {
            // This is a future card
            index = atoi(c + 1);
            if (index >= DECK_SIZE)
            {
                fprintf(stderr, "Invalid card index.\n");
                continue;
            }
            card_p = add_empty_card(hand);
            if (card_p == NULL)
            {
                fprintf(stderr, "Failed to add future card to hand.\n");
                continue;
            }
            add_future_card(fc, index, card_p);
        }
        else
        {
            card = card_from_letters(c[0], c[1]);
            if (!is_card_valid(card))
            {
                fprintf(stderr, "Invalid card.\n");
                continue;
            }
            add_card_to(hand, card);
        }
    }
    return hand;
}

deck_t ** read_input(FILE * f, size_t * n_hands, future_cards_t * fc)
/*
   This function reads the input from f. The input file has one hand per line 
   (a hand is of type deck_t). A deck_t type deck is allocated for each hand
   and it is placed into an array of pointers to deck_t decks, which is 
   returned. The number of decks is passed back trough the n_hands pointer.
   
   For any future future cards (?0, ?1, ...) in the deck, the add_empty_card is 
   used to create a placeholder in the hand. The add_future_card function is 
   used later to make sure the hand is updated correctly when cards are drawn 
   later. 
   
   The code assumes that a poker hand has AT LEAST 5 cards in it. If there are 
   fewer than 5 cards, a useful error message is printed before exit.
*/
{
    deck_t **hands = NULL;
    deck_t **new_hands = NULL;
    deck_t *new_hand = NULL;

    char *line = NULL;
    char *trimmed = NULL;
    size_t size = 0;
    ssize_t length = 0;

    while ((length = getline(&line, &size, f)) > 0)
    {
        trimmed = trim_hand(line);
        if (trimmed == NULL) continue;
        new_hand = hand_from_string(trimmed, fc);
        if (new_hand->n_cards < 5)
        {
            fprintf(stderr, "Not enough cards in hand.\n");
            free_deck(new_hand);
            for (size_t i = 0; i < *n_hands; ++i)
            {
                free_deck(hands[i]);
            }
            free(hands);
            return NULL;
        }
        new_hands = realloc(hands, sizeof(*hands) * (*n_hands + 1));
        if (new_hands == NULL)
        {
            fprintf(stderr, "Failed to allocate memorfy for hand. Error: %d\n", errno);
            free_deck(new_hand);
            free_decks(hands, *n_hands);
            *n_hands = 0;
            return NULL;
        }
        hands = new_hands;
        hands[*n_hands] = new_hand;
        ++*n_hands;
        free(trimmed);
    }
    if (line != NULL)
    {
        free(line);
    }
    return hands;
}

card_t card_from_num(unsigned c)
{
  card_t temp;
  temp.value = c % 13 + 2;
  temp.suit = c / 13;
  assert_card_valid(temp);
  return temp;
}

deck_t *generate_new_deck()
{
    deck_t *deck = initialize_deck();
    if (deck == NULL)
    {
      return NULL;
    }
    deck->cards = malloc(sizeof(*deck->cards) * DECK_SIZE);
    if (deck->cards == NULL)
    {
      fprintf(stderr, "Failed to allocate memory for cards. Error: %d\n", errno);
      free(deck);
      return NULL;
    }
    card_t card;
    for (int i = 0; i < DECK_SIZE; ++i)
    {
      card = card_from_num(i);
      deck->cards[i] = malloc(sizeof(*deck->cards[i]));
      if (deck->cards[i] == NULL)
      {
        fprintf(stderr, "Failed to allocate memory for card. Error: %d\n", errno);
        for (int j = 0; j < i; ++j)
        {
          free(deck->cards[j]);
        }
        free(deck->cards);
        free(deck);
        return NULL;
      }
      deck->cards[i]->suit = card.suit;
      deck->cards[i]->value = card.value;
    }
    deck->n_cards = DECK_SIZE;
    return deck;
}

void future_cards_from_deck(deck_t * deck, future_cards_t * fc)
{
    int index = 0;
    
    for (size_t i = 0; i < fc->n_decks; ++i)
    {
        for (size_t j = 0; j < fc->decks[i].n_cards; ++j)
        {
            fc->decks[i].cards[j]->suit = deck->cards[index]->suit;
            fc->decks[i].cards[j]->value = deck->cards[index]->value;
        }
        ++index;
    }
}

void free_deck(deck_t * deck)
{
/*   Frees the memory allocated to a deck of cards. */
  if (deck == NULL) return;
  for (int i = 0; i < deck->n_cards; ++i)
  {
    free(deck->cards[i]);
  }
  free(deck->cards);
  free(deck);
}

void print_hand(deck_t * hand)
{
  size_t n_cards = hand->n_cards;
  size_t last = n_cards - 1;
  card_t **cards = hand->cards;
  for (int i = 0; i < n_cards; ++i)
  {
    print_card(*cards[i]);
    if (i < last) printf(" ");
  }
}

void free_future_cards(future_cards_t *fc)
{
    if (fc == NULL) return;
    for (int i = 0; i < fc->n_decks; ++i)
    {
        if (fc->decks[i].cards != NULL)
        {
            free(fc->decks[i].cards);
        }
    }
    if (fc->decks != NULL)
    {
        free(fc->decks);
    }
    free(fc);
}

int cards_equal(card_t card1, card_t card2)
{
  return card1.value == card2.value && card1.suit == card2.suit;
}

int deck_contains(deck_t * d, card_t c)
{
  for (int i = 0; i < d->n_cards; ++i)
  {
    if (cards_equal(*d->cards[i], c)) return 1;
  }
  return 0;
}
