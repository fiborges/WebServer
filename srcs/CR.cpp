/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CR.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: brolivei <brolivei@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 14:51:17 by brolivei          #+#    #+#             */
/*   Updated: 2024/07/05 17:37:38 by brolivei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CR.hpp"

CR::CR() {}

CR::~CR() {}

CR::CR(int ClientSocket)
{
	this->ClientSocket_ = ClientSocket;
}

bool	CR::ItIsChunked(std::string& Request)
{
	std::string	ToFind = "Transfer-Encoding: chunked";

	if (Request.find(ToFind))
		return (true);
	else
		return (false);
}

bool	CR::TheRequestIsFinished()
{
	size_t	FirstPos = this->FullRequest_.find("\r\n\r\n");

	if (FirstPos == std::string::npos)
		return false;
	else if (this->FullRequest_.find("\r\n\r\n", FirstPos + 4) == std::string::npos)
		return false;
	return true;
}

void	CR::HandleRequest()
{
	char	buffer[4096];
	int		bytesRead;

	while ((bytesRead = recv(this->ClientSocket_, buffer, 4095, 0)) > 0)
	{
		buffer[bytesRead] = '\0';
		//std::cout << "Chunked Received:\n" << buffer << std::endl;
		this->FullRequest_.append(buffer);
		if (TheRequestIsFinished() == true)
			break;
	}
	if (bytesRead < 0)
		std::cerr << "Error on recv\n";

	std::cout << "TOTAL REQUEST:\n" << this->FullRequest_ << std::endl;
	ProcessChunked();
}

int	ConvertToDeci(std::string hex)
{
	int	x = 0;
	int	NuSize = 0;
	char	Hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	while (hex[NuSize])
		NuSize++;

	NuSize--;

	for (int i = 0; hex[i]; i++)
	{
		if (hex[i] >= 'a' && hex[i] <= 'z')
			hex[i] -= 32;
	}

	for (int i = 0; hex[i]; i++)
	{
		for (int j = 0; j <= 16; j++)
		{
			if (hex[i] == Hex[j])
			{
				x += (j * std::pow(16, NuSize));
				NuSize--;
				break;
			}
		}
	}

	return (x);
}



std::string	CR::ProcessChunked()
{
	size_t		PosBegChunk = this->FullRequest_.find("\r\n\r\n") + 4;
	std::string	SizeOfChunkInHex;
	int			SizeOfChunkInDec;

	this->CleanedRequest.append(this->FullRequest_, 0, PosBegChunk);

	std::cout << "FirstPartOfCleanedRequest:\n" << this->CleanedRequest << "$\n";

	size_t	InicioDosCHUNKES = this->FullRequest_.find("\r\n", PosBegChunk) + 2;

	while (this->FullRequest_[InicioDosCHUNKES] != '\r')
		SizeOfChunkInHex += this->FullRequest_[InicioDosCHUNKES++];
	SizeOfChunkInDec = ConvertToDeci(SizeOfChunkInHex);


	InicioDosCHUNKES += 2;
	while (SizeOfChunkInDec != 0)
	{
		std::string	NewPiece;
		//size_t	ChunkFinalPos = this->FullRequest_.find("\r\n", InicioDosCHUNKES);

		while (NewPiece.size() != static_cast<size_t>(SizeOfChunkInDec))
			NewPiece += this->FullRequest_[InicioDosCHUNKES++];

		this->CleanedRequest.append(NewPiece);

		InicioDosCHUNKES += 2;

		SizeOfChunkInHex = "";

		while (this->FullRequest_[InicioDosCHUNKES] != '\r')
			SizeOfChunkInHex += this->FullRequest_[InicioDosCHUNKES++];
		SizeOfChunkInDec = ConvertToDeci(SizeOfChunkInHex);

		InicioDosCHUNKES += 2;
	}

	std::cout << "CleandedRequest:\n" << this->CleanedRequest << std::endl;

	return (this->CleanedRequest);
}
